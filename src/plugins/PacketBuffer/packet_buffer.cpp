/**
 * This plugin buffers a group of packets in in-memory cyclic buffers and sends them
 * in chunked groups. By deliberately deciding when we send our packets we obtain
 * the ability to ensure our packet losses are minimal by buffering our packets to
 * send only when they are able to get to the other side.
 */

#include "../npsgate_plugin.hpp"
#include "../logger.hpp"
#include <unistd.h>
#include <stdlib.h>

class PacketBuffer : public NpsGatePlugin {

    private:
        Packet* buf1_ptr, buf2_ptr;
        Packet* cyclicBuffer1, cyclicBuffer2;

    public:
        bool init() {
            LOG_INFO("PacketBuffer plugin starting initialization.\n");

            parse_config();

            // we first malloc() our cyclic buffers.
            (Packet*) cyclicBuffer1 = malloc(sizeof(Packet) * 400);
            (Packet*) cyclicBuffer2 = malloc(sizeof(Packet) * 400);

            return true;
        }
        // This is going to handle the sending logic, ie, buffering the packets if
        // the buffer isn't full and then sending them once it is. 
        bool process_packet(Packet* p) {
            // do something here
            return true;
        }

        // This will have to do something re: switching the outputs because i don't
        // even know what the fuck the ipoutput plugin sends
        bool process_message(Message* m) {
            // do something here
            return true;
        }

        bool main() {
            message_loop();
            return true;
        }

        void parse_config() {
            try {
                const Config* config = get_config();
                const Setting& root = config->getRoot();
                const Setting& conf_plugins = root["outputs"];

                LOG_INFO("Parsing %d config directives:\n", conf_plugins.getLength());

                for (int i=0; i < conf_plugins.getLength(); i++) {
                    const Setting& plugin_conf = conf_plugins[i];
                    string name;
                    int protocol;

                    // beyond here we actually have to have a conf file structure in mind
                    // so that we know what keys to look for and fail on not finding.
                    // see NFQueue plugin for implementation example.
                    // TODO: implement
                }
            } catch (SettingNotFoundException& ex) {
                return;
            }
        }
}

NPSGATE_PLUGIN_CREATE(PacketBuffer);
NPSGATE_PLUGIN_DESTROY();
