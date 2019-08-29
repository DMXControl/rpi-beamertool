// artnet_receiver.h

#ifndef ARTNET_RECEIVER_H
#define ARTNET_RECEIVER_H

#include <netinet/in.h>
#include <string>
#include <vector>
#include <thread>
#include "canvas/canvas_classes.h"

#define ART_NET_PORT 6454               // Art-Net Port
#define NET_BUFFER_LENGTH 1024          // packet reveive buffer
#define LED_SWITCH_AFTER_PACKETS_NUM 2  // change LED every _NUM_ packets

namespace Beamertool {

    // Network Interface Struct
    struct networkInterface {
        std::string name;                // Interface name
        unsigned long ip_address;        // Interface IP Address
        bool has_broadcast;              // has broadcast ip flag
        unsigned long broadcast_address; // broadcast address if this interface
    };

    class ArtnetReceiver {

    public:
        /**
         * standard constructor
         * @param screen            Pointer to the Canvas Manager
         * @param canvases_id       id of the canvas group managed by Artnet
         * @param universe          Universe the ArtNet Plugin shall listen to
         * @param subnet            Subnet the ArtNet Plugin shall listen to
         * @param dmx_start         DMX Start Address of the first Canvas
         * @param scale_multiplier  Multiplier for Scaling an Position
         * @param led_gpio_pin      GPIO Pin for Packet receive status LED
         */
        ArtnetReceiver(CanvasManager * screen, int canvases_id, unsigned int universe, unsigned int subnet, int dmx_start, float scale_multiplier, int led_gpio_pin);

        /**
         * destructor
         */
        ~ArtnetReceiver();

    private:
        /**
         * net listener function
         */
        void netListener();

        /**
         * artnet parser
         * @param len the length of the packet to parse
         * @return 0=artnet packet recveived -1=bad or no artnet packet
         */
        int parseArtNet(int len);

        /**
         * Set Canvases from DMX universe
         */
        void updateCanvasValues();

        /**
         * trim angels to range [0, 360)
         * @param angle angle to trim
         * @return angle in range [0, 360)
         */
        float correctAngleRange(float angle);

        /**
         * send the ArtPollReply
         */
        void sendArtPollReply();

        /**
         * update the list with local Interfaces
         */
        void getNetworkInterfacesList();

        /**
         * WARNING: TODO (not implemented, meanwhile eth0 wisll be choosen)
         *
         * choose my IP and the Broadcast IP for ArtPollReply by comparing the
         * list of local Network Interfaces and the client IP.
         * This is (in my opinion) the easiest way to get my IP Address,
         * respectively to get the IP Adress of the Interface whitch has
         * received an ArtNet packet
         * Alternatives:
         * - run Lister for each Local network interface, but i think it is
         *   safer to listen at INADDR_ANY to get all packets
         * - listen to raw UDP packets to get my IP Adress and thereof the
         *   broadcast adress of the related interface, but then i have tho
         *   parse the IP header of each received packet
         */
        void calcMyAndBroadcastIP(unsigned long client_ip);

        /**
         * init the LED output (if root)
         */
        void initLED();

        /**
         * set the led on or of (if root)
         * @param value 0=led off; 1=led on
         */
        void setLED(int value);

        /**
         * reset the LED output (if root)
         */
        void quitLED();


        std::thread net_listener_thread;            // net listener thread
        bool quit;                                  // shall quit status
        CanvasManager * screen;                     // The Canvas Manager
        int canvases_id;                            // Canvasgroup ID
        int artnet_universe;                        // Artnet Universe
        int artnet_subnet;                          // Artnet Subnet
        int dmx_start;                              // DMX start
        float scale_multiplier;                     // scale multiplier
        unsigned char dmx512[512];                  // dmx512 universe
        int s;                                      // socket
        struct sockaddr_in my_receive_address;      // my Socket Address
        struct sockaddr_in client_address;          // clients Socket Address
        char buffer[NET_BUFFER_LENGTH];             // network packet buffer
        std::vector<networkInterface> interfaces;   // local network interface list
        unsigned long my_ip_address;                // my ip address (for ArtPollReply)
        unsigned long send_broadcast_address;       // send  ArtPollReply to
        int led_gpio_pin;                           // Packet receive status LED pin
        int led_status;                             // LED status (on/off)
        int led_packet_counter;                     // packets received since last LED change
        bool is_root;                               // program runs with root privileges flag
    };
}

#endif // ARTNET_RECEIVER_H
