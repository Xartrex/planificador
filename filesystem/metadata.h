
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


#include <stdint.h>

#define MAX_FN 48 //Cantidad máxima de ficheros, RNF1
#define MAX_NL 33 //Longitud máxima de nombre de fichero, RNF2 32 caracteres + 1 byte \0 por codificacion ascii
#define MAX_UB 240 //Cantidad máxima de bloques usables 48 inodos * 5 bloques/inodo = 240 bloques como máximo
// #define MAX_FS 10240 //Tamaño máximo de fichero, RNF3 (MAX_FILE_SIZE en filesystem.h)
#define MIN_DS 471040 //Tamaño mínimo de disco
#define MAX_DS 614400 //Tamaño máximo de disco
#define MAX_BI 5 //Numero máximo de bloques de datos asociados
#define FIRST_DATA_BLOCK 2 // primer bloque de datos 0 y 1 superbloque + inodos
#define INODE_SIZE 64 // 64 bytes para cada bloque
#define TIPO_FILE 1 // tipo de inodo: FICHERO
#define TIPO_LINK 2 // tipo de inodo: ENLACE

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))

static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_) bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}


struct inode_struct { // Queremos que ocupe 64 bytes para meetr 48 de ellos en 3KB
	char name[MAX_NL]; //El nombre del fichero, con tamaño máximo
	uint8_t type; //El tipo de inodo, 1 fichero, 2 enlace, 0 directorio (no se usa directorio)
	uint16_t size; //Tamaño del fichero en bytes asociado a este inodo
	uint32_t CRC; //Variable para comprobar integridad
	uint8_t num_block[MAX_BI]; //numero de bloque de cada uno de los 5 bloques que puede ocupar. -1 si vacio
	uint8_t link; //file al que apunta para inodos tipo enlace. num de enlaces para inodos tipo file
	char padding [18]; // llevamos bytes: 33 + 1 + 2 + 4 + 5 + 1 = 46B; 64B - 45B = 18B padding
};
typedef struct inode_struct inode;


struct super_block_struct{ //Le reservamos 1/2 bloque, tenemos 1KB para rellenar
	// nos sobra
	int magic_num; 
	char inode_map[MAX_FN]; //mapa de inodos, 0 libre, 1 usado
	char block_map[MAX_UB]; //mapa de bloques, 0 libre, 1 usado
	int numBlocks; // tamaño de disco //
};
typedef struct super_block_struct super_block;


struct inode_x_struct{
	int position; //posicion R/W
	int open;  // 0 cerrado, 1 abierto, 2 abierto con integridad
};
typedef struct inode_x_struct inode_x;
