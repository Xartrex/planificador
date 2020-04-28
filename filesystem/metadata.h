
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
#define MAX_NL 33 //Longitud máxima de nombre de fichero, RNF2 32 caracteres + 1 byte \0 por codificacion ascii
#define MAX_UB 240 //Cantidad máxima de bloques usables 48 inodos * 5 bloques/inodo = 240 bloques como máximo
#define MAX_FS 10240 //Tamaño máximo de fichero, RNF3
#define MAX_BS 2048 //Tamaño máximo de bloque 2KB, RNF4
#define MIN_DS 471040 //Tamaño mínimo de disco
#define MAX_DS 614400 //Tamaño máximo de disco
#define MAX_BI 5 //Numero máximo de bloques de datos asociados 


struct super_block{ //ocupa 1 bloque, tenemos 2KB para rellenar
	// 16 + 48*8 + 8 + 240*8 + 48*64 = 
	uint16_t magic_num; //hemos usado uint16_t para dar margen a escribir más numeros
	char inode_map [MAX_FN]; //mapa de inodos, 0 libre, 1 usado
	char block_map [MAX_UB]; //mapa de bloques, 0 libre, 1 usado
	uint8_t num_blocks; // tamaño de disco - 1(superbloque)
	
	struct inode arr_inodes [MAX_FN]; //array de inodos
};

typedef struct inode_x{
	int position; //posicion R/W
	int open;  // 0 cerrado, 1 abierto
};

//El inodo ocupa 512/8 = 64B incluido el padding
typedef struct inode{ //llevamos bits: 264 + 8 + 16 + 32 + 8  = 328 bits
	char name [MAX_NL]; //El nombre del fichero, con tamaño máximo
	uint8_t type;	//El tipo de inodo, 0 fichero, 1 directorio
	uint16_t size; //Tamaño del fichero en bytes asociado a este inodo
	uint32_t CRC; //Variable para comprobar integridad
	uint8_t num_blocks [MAX_BI]; //cantidad de bloques de datos que ocupa el fichero asociado a este inodo
	char padding [23]; // 512 - 328 = 184 / 8 = 23
}; 

