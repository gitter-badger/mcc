#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "bot.h"
#include "protocol.h"
#include "json.h"
#include "api.h"

typedef struct bot_globals {
    int status;
} bot_globals_t;

void dig(bot_t *bot, int x, int y, int z) {
    position_t target = ((x & 0x3FFFFFF) << 38) |
                        ((y & 0xFFF) << 26) |
                        (z & 0x3FFFFFF);
    send_play_serverbound_animation(bot);
    send_play_serverbound_player_dig(bot, 0, target, 1);
    send_play_serverbound_animation(bot);
//    msleep(1000);
//    send_play_serverbound_player_dig(bot, 2, target, 1);
}

void decode_chat_json(char *raw_json, char **msg, char **sender_name)
{
    json_value *json_data = json_parse((json_char *)raw_json, strlen(raw_json));

    for (int i = 0; i < json_data->u.object.length; i++) {
        char * name = json_data->u.object.values[i].name;
        if (strcmp(name, "with") == 0) {
            json_value * arr = json_data->u.object.values[i].value;
            // Iterate over array data['with'].
            for (int j = 0; j < arr->u.array.length; j++) {
                json_value * curr = arr->u.array.values[j];
                switch (curr->type) {
                // The sender is in the object data.
                case json_object:
                    ;
                    for (int k = 0; k < curr->u.object.length; k++) {
                        char * name = curr->u.object.values[k].name;
                        if (strcmp(name, "text") == 0) {
                            // sender_name = data['with'][0]['text']
                            json_value * sender = curr->u.object.values[k].value;
                            *sender_name = malloc(sender->u.string.length + 1);
                            memcpy(*sender_name, sender->u.string.ptr,
                                   sender->u.string.length + 1);
                            break;
                        }
                    }
                    break;
                // The actual message is the string.
                case json_string:
                    ;
                    // msg = data['with'][1]
                    *msg = malloc(curr->u.string.length + 1);
                    memcpy(*msg, curr->u.string.ptr, curr->u.string.length + 1);
                    break;
                default:
                    printf("invalid JSON message error.");
                    break;
                }

            }
            // Short circuit on key == 'with'.
            break;
        }
    }
    json_value_free(json_data);
}

void exec(bot_t *bot, char *command, char *strargs) {
    if (strcmp(command, "dig") == 0) {
        send_play_serverbound_chat(bot, command);
        send_play_serverbound_chat(bot, strargs);
        char **saveptr = calloc(1, sizeof(char *));
        int x = (int)strtol(strtok_r(strargs, " ", saveptr), NULL, 0);
        int y = (int)strtol(strtok_r(*saveptr, " ", saveptr), NULL, 0);
        int z = (int)strtol(strtok_r(*saveptr, " ", saveptr), NULL, 0);
        free(saveptr);
        printf("DIG @ (%d, %d, %d)\n", x, y, z);
        dig(bot, x, y, z);
    } else {
        printf("help.\n");
    }
}

void chat_handler(bot_t *bot, void *vp)
{
    play_clientbound_chat_t *p =
        (play_clientbound_chat_t *)vp;
    char *msg = NULL;
    char *sender = NULL;
    decode_chat_json(p->json, &msg, &sender);
    // Ensure that we do not echo ourselves
    if (msg && strcmp(sender, bot->name)) {
        // Commands to bots start with a backslash.
        // Only operate on non-empty commands.
        if (msg[0] == '\\' && msg[1]) {
            // Parse space delimited tokens.
            char **saveptr = calloc(1, sizeof(char *));
            char *command = strtok_r(msg+1, " ", saveptr);
            exec(bot, command, *saveptr);
            //printf("COMMAND: <%s> %s\n", sender, command);
            free(saveptr);
        } else {
            printf("CHAT: <%s> %s\n", sender, msg);
        }
    }
    free(sender);
    free(msg);
}

void slave_main(void *vbot)
{
    bot_t *bot = (bot_t *)vbot;
    msleep(1000);
    send_play_serverbound_item_change(bot, 0);

    while(1) {
        dig(bot, (int)bot->x, (int)bot->y-1, (int)bot->z);
        // Move update
        send_play_serverbound_player_move(bot, bot->x, bot->y, bot->z, 1);

        // Respawn
        //send_play_serverbound_player_status(bot, 0);
        
        msleep(300);
        send_play_serverbound_player(bot, 0);
        send_play_serverbound_animation(bot);
        msleep(300);
        send_play_serverbound_player(bot, 0);
        send_play_serverbound_animation(bot);
        msleep(300);
        send_play_serverbound_player(bot, 0);
        send_play_serverbound_animation(bot);
        msleep(300);
        send_play_serverbound_player(bot, 0);
        send_play_serverbound_animation(bot);

    }
}

bot_t *init_slave(char *name, char *server_name, int port)
{
    bot_t *bot = init_bot(name, *slave_main);
    bot->item = calloc(1, sizeof(bot_globals_t));

    register_defaults(bot);
    register_event(bot, PLAY, 0x02, chat_handler);

    login(bot, server_name, port);

    return bot;
}
