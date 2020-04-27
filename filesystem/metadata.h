
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	Last revision 01/04/2020
 *
 */

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

#include <stdint.h>

#define MAX_FN 48 //Cantidad máxima de ficheros, RNF1
#define MAX_NL 32 //Longitud máxima de nombre de fichero, RNF2

//#define MAX_FS 10240 //Tamaño máximo de fichero, RNF3
/*#define MAX_BS 2048 //Tamaño máximo de bloque, RNF4
#define MIN_DS 471040 //Tamaño mínimo de disco
#define MAX_DS 614400 //Tamaño máximo de disco
*/


struct super_block{
	//num_magico, tamaño de disco

};

typedef struct inode{
	//tipo, tamaño, CRC,bloque max 5(mapa bits)
	char iname [MAX_NL]; //El nombre del fichero, con tamaño máximo
	uint8_t type;	//El tipo de inodo, 0 fichero, 1 directorio
	uint16_t size; //Tamaño del fichero en bytes
	uint32_t CRC; //Variable para comprobar integridad//////////////////////////////////////////////////
};

