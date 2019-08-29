// artnet_receiver.cpp

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include "artnet_receiver.h"
#include "canvas/canvas_manager.h"
#include "canvas/canvas.h"

using namespace std;
using namespace Beamertool;

ArtnetReceiver::ArtnetReceiver(CanvasManager * screen, int canvases_id, unsigned int universe, unsigned int subnet, int dmx_start, float scale_multiplier, int led_gpio_pin) {

    // save parameters
    this->screen = screen;
    this->canvases_id = canvases_id;
    this->artnet_universe = universe;
    this->artnet_subnet = subnet;
    this->dmx_start = dmx_start;
    this->scale_multiplier = scale_multiplier;
    this->led_gpio_pin = led_gpio_pin;
    this->led_status = 0;
    this->led_packet_counter = 0;

    // Init Socket
    this->s = socket (AF_INET, SOCK_DGRAM , IPPROTO_UDP);
    if (this->s < 0) {
        printf ("init_net: Kann Socket nicht öffnen ...(%s)\n", strerror(errno));
        exit (EXIT_FAILURE);
    }

    // allow Broadcast
    int broadcastPermission = 1;
    if (setsockopt(this->s, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission)) < 0) {
        printf ("setsockopt() failed");
        exit (EXIT_FAILURE);
    }

    // listen at ip/port
    this->my_receive_address.sin_family = AF_INET;
    this->my_receive_address.sin_addr.s_addr = htonl(INADDR_ANY);
    this->my_receive_address.sin_port = htons(ART_NET_PORT);

    // bin port
    if(::bind(this->s, (struct sockaddr *)&this->my_receive_address, sizeof(this->my_receive_address)) < 0) {
        printf ("ArtnetReceiver: bind(): Kann Portnummern %d nicht binden (%s)\n", ART_NET_PORT, strerror(errno));
        exit (EXIT_FAILURE);
    }

    // check for root
    this->is_root = false;
    if (getuid() == 0) {
        this->is_root = true;
    }

    // init LED
    initLED();
    setLED(1);

    // Start net listener
    memset(&this->client_address, '\0', sizeof(this->client_address));
    this->quit = false;
    net_listener_thread = thread(&ArtnetReceiver::netListener, this);

    // send ArtPollReply on powerup
    sendArtPollReply();
}

ArtnetReceiver::~ArtnetReceiver() {

    // stop listening
    this->quit = true;
    shutdown(this->s,0);
    net_listener_thread.join();
    close(this->s);

    // poweroff led
    setLED(0);
    quitLED();
}

void ArtnetReceiver::netListener() {
    while (!this->quit) {
        // reset buffer
        memset (this->buffer, 0, NET_BUFFER_LENGTH);

        // get packet
        socklen_t len = sizeof (this->client_address);
        int n = recvfrom ( this->s, this->buffer, NET_BUFFER_LENGTH, 0 /*alternativ: MSG_DONTWAIT*/, (struct sockaddr *) &this->client_address, &len );

        // parse Artnet and set LED
        if(parseArtNet(n) == 0) {
            this->led_packet_counter++;
            if (this->led_packet_counter > LED_SWITCH_AFTER_PACKETS_NUM) {
                if (this->led_status == 0) {
                    setLED(1);
                } else {
                    setLED(0);
                }
            }
        }
    }
}

int ArtnetReceiver::parseArtNet(int len) {
    // Check Header and Opcode
    if (len >=10 && strncmp(&this->buffer[0], "Art-Net", 8) == 0) {
        int opcode = (this->buffer[9] << 8) + this->buffer[8];
        switch (opcode) {
            // OpPoll
            case 0x2000:
                sendArtPollReply();
                return 0;
                break;
            // OpDmx
            case 0x5000:
                if (len > 18) {
                    // this->buffer [10+11] = Art-Net Rev Number
                    //short sequence = this->buffer[12]; // Reihenfolge der Art-Net Pakete
                    //short physical = this->buffer[13];
                    //short subuni = this->buffer[14]; // _
                    //short net = this->buffer[15];    // -> zusammen universe in Wireshark, 15bitPort-Adress nach Art-Net Specification
                    int subnet = (this->buffer[14] & 0xF0) >> 4;  // _
                    int universe = this->buffer[14] & 0xF;        // -> so sendet DMXControl Art-Net Plugin (DMXC V. 2.12)
                    if (subnet == this->artnet_subnet && universe == this->artnet_universe) {
                        int dmxdata_len = ((this->buffer[16]<<8) | this->buffer[17]);
                        if(dmxdata_len >=2 && dmxdata_len <= 512) {
                            for (int i=0;i<dmxdata_len;i++) {
                                this->dmx512[i] = this->buffer[18+i];
                            }
                            this->updateCanvasValues();
                        }
                    }
                }
                return 0;
                break;
            default:
                break;
        }
    }
    return -1;
}

void ArtnetReceiver::updateCanvasValues() {

    vector<Canvas*> canvases = this->screen->getCanvasGroup(this->canvases_id);

    for(unsigned int i=0; i<canvases.size(); ++i) {
        canvases[i]->setContentData(this->dmx512[this->dmx_start + i * 20 - 1 + 0]);

        canvases[i]->setNumber((unsigned int)(this->dmx512[this->dmx_start +i * 20 - 1 + 7] / 4));
        canvases[i]->setZoom(((float) this->dmx512[this->dmx_start +i * 20 - 1 + 9]) * this->scale_multiplier / 255.f);

        canvases[i]->setScalingX( (float) this->dmx512[this->dmx_start +i * 20 - 1 + 4] * this->scale_multiplier / 255.f);
        if (this->dmx512[this->dmx_start +i * 20 - 1 + 5] > 0) {
            canvases[i]->setScalingY(((float) this->dmx512[this->dmx_start +i * 20 - 1 + 5] / 255.f) / screen->getScreenRatio());
        } else {
            canvases[i]->setScalingY(-1.0f);
        }

        canvases[i]->setPositionX(((float) this->dmx512[this->dmx_start +i * 20 - 1 + 2] - 128.f) * this->scale_multiplier * 2.f / 256.f);
        canvases[i]->setPositionY((((float) this->dmx512[this->dmx_start +i * 20 - 1 + 3] - 128.f) * this->scale_multiplier * 2.f / 256.f) / screen->getScreenRatio());

        canvases[i]->setR((float) this->dmx512[this->dmx_start +i * 20 - 1 + 10] / 255.0f);
        canvases[i]->setG((float) this->dmx512[this->dmx_start +i * 20 - 1 + 11] / 255.0f);
        canvases[i]->setB((float) this->dmx512[this->dmx_start +i * 20 - 1 + 12] / 255.0f);
        canvases[i]->setAlpha((float) this->dmx512[this->dmx_start +i * 20 - 1 + 1] / 255.0f);

        int dmx_rot = this->dmx512[this->dmx_start +i * 20 - 1 + 6];
        int dmx_speed = this->dmx512[this->dmx_start +i * 20 - 1 + 15];
        int dmx_rot_all = this->dmx512[this->dmx_start +i * 20 - 1 + 8];
        int dmx_speed_all = this->dmx512[this->dmx_start +i * 20 - 1 + 16];

        if (dmx_rot > 0) {
            canvases[i]->setRotation( -(((float)dmx_rot) / 256.0f) * 360.0f);
            canvases[i]->setSpeed(0.0f);
        } else {
            if (dmx_speed == 127) {
                canvases[i]->setRotation(0.0f);
                canvases[i]->setSpeed(0.0f);
            } else if(dmx_speed > 127) {
                canvases[i]->setSpeed(correctAngleRange( -((float)((dmx_speed - 127) * ((dmx_speed - 127) / 8))) / 65536.f * 360.f ));
            } else {
                canvases[i]->setSpeed(correctAngleRange( ((float)(((dmx_speed - 127) * ((dmx_speed - 127) / 8)))) / 65536.f * 360.f ));
            }
        }

        if (dmx_rot_all > 0) {
            canvases[i]->setRotationAll( -(((float)dmx_rot_all) / 256.0f) * 360.0f);
            canvases[i]->setSpeedAll(0.0f);
        } else {
            if (dmx_speed_all == 127) {
                canvases[i]->setRotationAll(0.0f);
                canvases[i]->setSpeedAll(0.0f);
            } else if(dmx_speed_all > 127) {
                canvases[i]->setSpeedAll(correctAngleRange( -((float)((dmx_speed_all - 127) * ((dmx_speed_all - 127) / 8))) / 65536.f * 360.f ));
            } else {
                canvases[i]->setSpeedAll(correctAngleRange( ((float)(((dmx_speed_all - 127) * ((dmx_speed_all - 127) / 8)))) / 65536.f * 360.f ));
            }
        }

        canvases[i]->setShutterSpeed(this->dmx512[this->dmx_start +i * 20 - 1 + 14]);
   }
}

float ArtnetReceiver::correctAngleRange(float angle) {
    while(angle >= 360.f) {
        angle -= 360.f;
    }
    while(angle < 0.f) {
        angle += 360.f;
    }
    return angle;
}

void ArtnetReceiver::sendArtPollReply() {

    calcMyAndBroadcastIP(ntohl(this->client_address.sin_addr.s_addr));

    char buffer[239];

    // ID
    buffer[  0] = 'A';
    buffer[  1] = 'r';
    buffer[  2] = 't';
    buffer[  3] = '-';
    buffer[  4] = 'N';
    buffer[  5] = 'e';
    buffer[  6] = 't';
    buffer[  7] = 0x0;
    // OpCode
    buffer[  8] = 0x00;
    buffer[  9] = 0x21;
    // IP Address
    buffer[ 10] = (this->my_ip_address>>24)&0xFF;
    buffer[ 11] = (this->my_ip_address>>16)&0xFF;
    buffer[ 12] = (this->my_ip_address>>8)&0xFF;
    buffer[ 13] = this->my_ip_address&0xFF;
    // Port
    buffer[ 14] = 0x36;
    buffer[ 15] = 0x19;
    // VersInfoH
    buffer[ 16] = 0;//?
    // VersInfo
    buffer[ 17] = 0;//?
    // NetSwitch
    buffer[ 18] = 0x0;
    // SubSwitch
    buffer[ 19] = this->artnet_subnet;
    // OemHi
    buffer[ 20] = 0x00;
    // Oem
    buffer[ 21] = 0xFF;
    // Ubea Version
    buffer[ 22] = 0x0;
    // Status1
    buffer[ 23] = 0x0;
    // EstaManLo
    buffer[ 24] = 0;//?
    // EstaManHi
    buffer[ 25] = 0;//?
    // ShortName
    for(int i=26; i<43; ++i) {
        buffer[i] = 0x0;
    }
    strcpy(&buffer[26], "RPI Beamertool");
    // LongName
    for(int i=44; i<=107; ++i) {
        buffer[i] = 0x0;
    }
    strcpy(&buffer[44], "Raspberry Pi Beamertool");
    // NodeReport
    for(int i=108; i<=171; ++i) {
        buffer[i] = 0x0;
    }
    strcpy(&buffer[108], "#0001 [0000] Status: OK!");
    // NumPortsHi
    buffer[172] = 0x0;
    // NumPortsLo
    buffer[173] = 0x1;
    // PortTypes
    buffer[174] = 0x80;
    buffer[175] = 0x00;
    buffer[176] = 0x00;
    buffer[177] = 0x00;
    // GoodInput
    buffer[178] = 0x0;
    buffer[179] = 0x0;
    buffer[180] = 0x0;
    buffer[181] = 0x0;
    // GoodOutput
    buffer[182] = 0x80;
    buffer[183] = 0x0;
    buffer[184] = 0x0;
    buffer[185] = 0x0;
    // SwIn
    buffer[186] = 0x0;
    buffer[187] = 0x0;
    buffer[188] = 0x0;
    buffer[189] = 0x0;
    // SwOut
    buffer[190] = this->artnet_universe;
    buffer[191] = 0x0;
    buffer[192] = 0x0;
    buffer[193] = 0x0;
    // SwVideo
    buffer[194] = 0x0;
    // SwMacro
    buffer[195] = 0x0;
    // SwRemote
    buffer[196] = 0x0;
    // Spare
    buffer[197] = 0x0;
    buffer[198] = 0x0;
    buffer[199] = 0x0;
    // Style
    buffer[200] = 0x00;
    // MAC
    buffer[201] = 0x0;
    buffer[202] = 0x0;
    buffer[203] = 0x0;
    buffer[204] = 0x0;
    buffer[205] = 0x0;
    buffer[206] = 0x0;
    // BindIp
    buffer[207] = 0;
    buffer[208] = 0;
    buffer[209] = 0;
    buffer[210] = 0;
    // BindIndex
    buffer[211] = 0;
    // Status2
    buffer[212] = 0x0;
    // Filler
    for(int i=213; i<=238; ++i) {
        buffer[i] = 0x0;
    }
    // ## END ArtPolLReply ##

    struct sockaddr_in send_to_addr;
    memset(&send_to_addr, '\0', sizeof(send_to_addr));
    send_to_addr.sin_family = AF_INET;
    send_to_addr.sin_port = htons(ART_NET_PORT);
    send_to_addr.sin_addr.s_addr = htonl(this->send_broadcast_address);

    sendto(this->s, buffer, sizeof(buffer), 0, (struct sockaddr*)&send_to_addr, sizeof(send_to_addr));
}

void ArtnetReceiver::calcMyAndBroadcastIP(unsigned long client_ip) {

    getNetworkInterfacesList();

    // TODO: calculate my IP and Broadcast IP from Interface List an client IP
    // as long as this is not implemented choose interfaces[0] witch should be eth0

    this->my_ip_address = this->interfaces[0].ip_address;
    if (this->interfaces[0].has_broadcast) {
        this->send_broadcast_address = this->interfaces[0].broadcast_address;
    } else {
        this->send_broadcast_address = 0x0;
    }
}

void ArtnetReceiver::getNetworkInterfacesList() {

    interfaces.clear();

    // get Interfaces
    struct ifaddrs * interfaces_address_list;
    getifaddrs(&interfaces_address_list);

    // iterate over interface list
    struct ifaddrs * list_entry = interfaces_address_list;
    while (list_entry != NULL) {

        struct sockaddr_in * entry_address = (struct sockaddr_in *) list_entry->ifa_addr;
        struct sockaddr_in * entry_broadcast_address = (struct sockaddr_in *) list_entry->ifa_broadaddr;

        // Fill interfaces Vector with AF_INET interfaces
        // Ignore lo (127.0.0.1) and if "eth0" exist add "eth0" as first entry
        if (entry_address->sin_family == AF_INET && strcmp(list_entry->ifa_name, "lo") != 0) {
            // parse interface
            networkInterface netintf;
            netintf.name = string(list_entry->ifa_name);
            netintf.ip_address = ntohl(entry_address->sin_addr.s_addr);
            if ((list_entry->ifa_flags & IFF_BROADCAST) > 0) {
                netintf.has_broadcast = true;
                netintf.broadcast_address = ntohl(entry_broadcast_address->sin_addr.s_addr);
            } else {
                netintf.has_broadcast = false;
                netintf.broadcast_address = 0x0;
            }

            // add to interfaces vector
            if (strcmp(list_entry->ifa_name, "eth0") == 0) {
                interfaces.insert(interfaces.begin(), netintf);
            } else {
                interfaces.push_back(netintf);
            }
        }
        list_entry = list_entry->ifa_next;
    }
    freeifaddrs(interfaces_address_list);

    // If interfaces vector is empty add zero entry
    if (interfaces.size() < 1) {
        networkInterface netintf;
        netintf.name = string("");
        netintf.ip_address = 0x0;
        netintf.has_broadcast = false;
        netintf.broadcast_address = 0x0;
        interfaces.push_back(netintf);
    }
}

void ArtnetReceiver::initLED() {
    if (this->is_root && this->led_gpio_pin >= 0) {
        FILE *ptr;

        ptr = fopen("/sys/class/gpio/export", "w");
        fprintf(ptr, "%d", this->led_gpio_pin);	// enable GPIO-Port
        fclose(ptr);

        char gpio[50];
        sprintf(gpio, "/sys/class/gpio/gpio%d/direction", this->led_gpio_pin);
        ptr = fopen(gpio, "w");
        fprintf(ptr, "out");  // set Port to output
        fclose(ptr);
    }
}

void ArtnetReceiver::setLED(int value) {
    if (this->is_root && this->led_gpio_pin >= 0) {
        int set_value;
        if (value >= 1) {
            set_value = 1;
            this->led_status = 1;
        } else {
            set_value = 0;
            this->led_status = 0;
        }

        FILE *ledptr;
        char gpio[50];
        sprintf(gpio, "/sys/class/gpio/gpio%d/value", this->led_gpio_pin);
        ledptr = fopen(gpio, "w");
        fprintf(ledptr, "%d", set_value);
        fclose(ledptr);
    }
}

void ArtnetReceiver::quitLED() {
    if (this->is_root && this->led_gpio_pin >= 0) {
        FILE *ptr;
        ptr = fopen("/sys/class/gpio/unexport", "w");
        fprintf(ptr, "%d", this->led_gpio_pin);
        fclose(ptr);
    }
}
