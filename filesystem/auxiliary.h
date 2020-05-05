
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	Last revision 01/04/2020
 *
 */

int namei(char *fileName);
void bfree(int bloque);
void ifree(int inodo);
int ialloc();
void readMetadataFromDisk();
void writeMetadaToDisk();
int bmap(int fd, int pos);

