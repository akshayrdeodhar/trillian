#ifndef CONFIG_H
#define CONFIG_H

/* REMEMBER: run 'make' after you modify config to see it's effects */

/* use 2 or 4. 6 takes too much time, odd depths give misguided moves. 2 is extremely weak */
#define CONF_DEPTH 2 

/* experimental feature in evaluation function- turn off with 0 */
#define CONF_SUPPORT_ATTACK 1

/* '#' makes the board look more like a chessboard, but then you will have to hunt for pieces. '.' is better */
#define CONF_BLACKSQUARE '.'

/* if nonzero, shows time taken by computer to make a move , position evaluation */
#define CONF_SPECS 1

/* the directory where you wish to put savegame files */
#define CONF_SAVES "./saves/"

/* file to be displayed as title screen- location wrt location of executable OR absolute location */
#define CONF_TITLE "./dat/titlescreen.txt"

/* default starting position to be  used when no arguement is specified. same advise as CONF_TITLE */

/* 1 is small and cute, 2 looks like a board. everything else is considered as 2 */
#define CONF_SIZE 2

/* unicode chess pieces might not work for your locale, turn off if horrible output seen on terminal */
#define CONF_UNICODE_PIECES 1


#endif
