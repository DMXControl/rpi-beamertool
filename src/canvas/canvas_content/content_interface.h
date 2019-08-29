// canvas/canvas_content/content_interface.h

#ifndef CONTENT_INTERFACE_H
#define CONTENT_INTERFACE_H

#include <string>
#include "../canvas.h"

namespace Beamertool {

    class ContentInterface {

    public:
        /**
         * destructor
         */
        virtual ~ContentInterface() = 0;

        /**
         * get Content Typ string
         * @return content type string
         */
        virtual std::string getContentTypeName() = 0;

        /**
         * get Content Ratio
         * @param content_data  content data
         * @param ratio         content ratio is set here
         */
        virtual void getContentRatio(int content_data, float * ratio) = 0;

        /**
         * set Content for Open GL
         * @param content_data  content data
         */
        virtual void setContent(int content_data) = 0;

    };
}

#endif // CONTENT_INTERFACE_H
