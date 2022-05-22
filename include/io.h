/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c);
void printc_colour(char c, char colour);
void printc_xy_colour(Byte x, Byte y, char c, char colour);
void printk(char *string);

#endif  /* __IO_H__ */
