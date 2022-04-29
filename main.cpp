#include "DSSimul.h"
//#include <unistd.h>
#include <assert.h>
#include <sstream>

int workFunction_Bully(Process *dp, Message m)
{
    string s = m.getString();
    NetworkLayer *nl = dp->networkLayer;
    if (!dp->isMyMessage("Bully", s)) return false;
    set<int> neibs = dp->neibs();
    /*if (dp->node == 5) {
        if (s == "Bully_Delay_req") {
            dp->networkLayer->send(dp->node, m.from, Message("Bully_Delay_resp"));
            printf("%3d: Bully_Delay_req by %3d\n", dp->node, m.from);
        }
        return true;
    }*/
    if (s == "Bully_Election" || s == "Bully_Election_Init") {
        printf("%3d: from %d: Start Election\n", dp->node, m.from);
        if (s != "Bully_Election_Init"){
            nl->send(dp->node, m.from, Message("Bully_Alive"));
            printf("%3d: sent to %3d Alive\n", dp->node, m.from);
        }
        if (dp->contextBully.started) {
            printf("%3d: from %d: Election already started, do nothing\n", dp->node, m.from);
            return true;
        }
        dp->contextBully.started = true;
        if (m.from < dp->node) {
            for (auto n: neibs) {
                if (n > dp->node) {
                    nl->send(dp->node, n, Message("Bully_Election"));
                    printf("%3d: sent to %3d Election\n", dp->node, n);
                }
            }
            nl->send(dp->node, dp->node, Message("Bully_Delay_req"));

        }


    } else if (s == "Bully_Alive") {
        dp->contextBully.finished = true;
        printf("%3d: Bully_Alive by %3d\n", dp->node, m.from);
    } else if (s == "Bully_Delay_req") {
        printf("%3d: Bully_Delay_req by %3d\n", dp->node, m.from);
        this_thread::sleep_for(chrono::milliseconds(500));
        dp->networkLayer->send(dp->node, m.from, Message("Bully_Delay_resp"));
    } else if (s == "Bully_Delay_resp") {
        if (dp->contextBully.started && !dp->contextBully.finished && dp->contextBully.time < 10) {
            dp->contextBully.time++;
            printf("%3d: Bully_Delay_resp: Update time by %3d\n", dp->node, m.from);
            nl->send(dp->node, dp->node, Message("Bully_Delay_req"));
        } else if (dp->contextBully.started && !dp->contextBully.finished) {
            dp->contextBully.finished = true;
            for (auto n: neibs) {
                nl->send(dp->node, n, Message("Bully_Coordinator"));
            }
            printf("%3d: I'm Coordinator\n", dp->node);
            dp->contextBully.coordinator = dp->node;
        } else {
            //printf("%3d: Something strange#2 by %3d\n", dp->node, m.from);
        }
    } else if (s == "Bully_Coordinator") {
        printf("%3d: %3d is Coordinator\n", dp->node, m.from);
        dp->contextBully.coordinator = m.from;
        dp->contextBully.finished = true;
    } else {
        printf("%3d: Something strange#1 by %3d\n", dp->node, m.from);
    }

    return true;
}


int main(int argc, char **argv)
{
    string configFile = argc > 1 ? argv[1] : "config.data";
    World w; 
    w.registerWorkFunction("Bully", workFunction_Bully);
    if (w.parseConfig(configFile)) {
        this_thread::sleep_for(chrono::milliseconds(3000000));
	} else {
        printf("can't open file '%s'\n", configFile.c_str());
    }
}

