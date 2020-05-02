
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	Last revision 01/04/2020
 *
 */

#include "filesystem/filesystem.h" // Headers for the core functionality
#include "filesystem/auxiliary.h"  // Headers for auxiliary functions
#include "filesystem/metadata.h"   // Type and structure declaration of the file system
#include <string.h>

super_block superbloque;
inode inodos[MAX_FN];
inode_x inodox[MAX_FN];
int mounted = 0;

/*
 * @brief Aux function which reads a byte from a memory position.
 * @return 	byte read
 */
char bitmap_getbyte(char *bitmap_) {
	char byte;
	for (int i = 0, e = 1; i < 8; i++, e *= 2) byte += e * bitmap_getbit(bitmap_, i); 
	return byte;
}

/*
 * @brief Aux function which writes a byte to a memory position.
 */
void bitmap_setbyte(char *bitmap_, char value) {
	char *byte = &value;
	for (int i = 0; i < 8; i++) bitmap_setbit(bitmap_, i, bitmap_getbit(byte, i)); 
}

/*
 * @brief Aux function which writes superblock and inodes from memory to disk.
 */
void writeMetadataToDisk() {
	char bloque0[BLOCK_SIZE];
	char bloque1[BLOCK_SIZE];
	// bloque 0 Meto superbloque al principio (tengo 1 KB de margen)
	//char *byte = (char *)&superbloque;
	memset(bloque0, 0, BLOCK_SIZE);
	memmove(bloque0, (char *)&superbloque, sizeof(superbloque));
/*	for (int i = 0; i < sizeof(superbloque); i++) {
		bloque0[i] = bitmap_getbyte(byte);
		byte++;
	}*/
	// bloque 0 Meto 16 inodos en la segunda mitad 16 * 64B = 1KB
	memmove(bloque0 + sizeof(superbloque), (char *)inodos, 16*sizeof(inode));
/*	byte = (char *)inodos;
	for (int i = 1024; i < 2048; i++) {
		bloque0[i] = bitmap_getbyte(byte);
		byte++;
	}*/
	// escribo bloque0 a disco
	bwrite(DEVICE_IMAGE, 0, bloque0);
	
	// bloque 1 Meto 32 inodos (del 16 al 47) 32 * 64B = 2KB
	memset(bloque1, 0, BLOCK_SIZE);
	memmove(bloque1, (char *)&(inodos[16]), 32*sizeof(inode));
/*	for (int i = 0; i < 2048; i++) {
		bloque1[i] = bitmap_getbyte(byte);
		byte++;
	}*/
	// escribo bloque1 a disco
	bwrite(DEVICE_IMAGE, 1, bloque1);
}

/*
 * @brief 	Aux function which reads superblock and inodes from disk to memory.
 */
void readMetadataFromDisk() {
	char bloque0[BLOCK_SIZE];
	char bloque1[BLOCK_SIZE];
	memset(bloque0, 0, BLOCK_SIZE);
	memset(bloque1, 0, BLOCK_SIZE);
	// leer bloque 0 de disco a superbloque
	bread(DEVICE_IMAGE, 0, bloque0);
	/*char *byte=(char *)&superbloque;
	for(int i =0; i<sizeof(superbloque); i++){
		bitmap_setbyte(byte, bloque0[i]);
		byte++;
	}*/
	memmove((char *)&superbloque, bloque0, sizeof(superbloque));
/*	byte=(char *)&inodos;
	for(int i =1024; i<2048; i++){
		bitmap_setbyte(byte, bloque0[i]);
		byte++;
	}*/
	memmove((char *)&inodos, bloque0 + sizeof(superbloque), 16* sizeof(inode));
	bread(DEVICE_IMAGE, 1, bloque1);

/*	for(int i =0; i<2048; i++){
		bitmap_setbyte(byte, bloque1[i]);
		byte++;
	}*/

	memmove((char *)&(inodos[16]), bloque1, 32* sizeof(inode));
}

/*
 * @brief 	Aux function which looks for a free inode.
 * @return 	0 if success, -1 otherwise.
 */
int ialloc(){
	// buscar un i-nodo libre
	for (int i = 0; i < MAX_FN; i++)
	{
		if (superbloque.inode_map[i] == 0) {
			// inodo ocupado ahora
			superbloque.inode_map[i] = 1;
			// valores por defecto en el i-nodo
			memset(&(inodos[i]), 0, sizeof( inode));
			// devolver identificador de i-nodo
			return i;
		}
	}
	return -1; // no hay hueco para mas inodos
}

/*
 * @brief 	Aux function which looks for a free block.
 * @return 	0 if success, -1 otherwise.
 */
int alloc(){
	char b[BLOCK_SIZE];
	int numBloquesDatos = superbloque.numBlocks-2;	//tamaño de disco (bloques) - 2 bloques de metadatos
	for (int i = 0; i < numBloquesDatos; i++) {
		if (superbloque.block_map[i] == 0) {
			// bloque ocupado ahora
			superbloque.block_map[i] = 1;
			// valores por defecto en el bloque
			memset(b, 0, BLOCK_SIZE);
			bwrite(DEVICE_IMAGE, i + FIRST_DATA_BLOCK, b);
			// devolver identificador del bloque
			return i;
		}
	}
	return -1; // no hay bloques libres  
}

/*
 * @brief 	Aux function which frees an inode
 */
void ifree(int inodo){
	superbloque.inode_map[inodo] = 0;
}

/*
 * @brief 	Aux function which frees an inode
 */
void bfree(int bloque){
	superbloque.block_map[bloque] = 0;
}


/*
 * @brief 	Aux function which returns inode_id from a filename
 * @return inode_id if success, -1 otherwise.
 */
int namei(char *filename){
	for(int i=0; i<MAX_FN; i++){
		if(strcmp(inodos[i].name, filename)==0)	return i;
	}
	return -1;
}

/*
 * @brief 	Aux function which gives block number where is the data of a file in a certain position
 * @return disk block number, -1 if error
 */
int bmap(int fd, int pos){
	if(pos>MAX_FILE_SIZE) return -1;

	int bloque = pos/BLOCK_SIZE;
	return inodos[fd].num_block[bloque];
}

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
	if (deviceSize < MIN_DS || deviceSize > MAX_DS){
		printf("Disk size %ldKB invalido (460KB - 600KB)\n", deviceSize / 1024); // añadido
		return -1;
	}
	superbloque.magic_num = 12345;
	superbloque.numBlocks = deviceSize / BLOCK_SIZE; //cambiado
	if (superbloque.numBlocks > MAX_UB + 2) superbloque.numBlocks = MAX_UB + 2; //
	for(int i = 0; i < MAX_FN; i++){
		superbloque.inode_map[i] = 0;
	} 
	for(int i = 0; i < MAX_UB; i++){
		superbloque.block_map[i] = 0;
	}
	for(int i = 0; i< MAX_FN; i++){
		memset(&(inodos[i]), 0, sizeof(inode)); // cambiado
	}
	for(int i = 0; i< MAX_FN; i++) {
		inodox[i].open = 0;
		inodox[i].position = 0;
	}
	// escribir los valores por defecto al disco
	writeMetadataToDisk();  

	return 0;
}


/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{
	if(mounted == 1) {
		printf("File system is already mounted\n");
		return -1;
	}
	readMetadataFromDisk();
	mounted = 1;
	return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	for(int i = 0; i < MAX_FN; i++){
		if(1 == inodox[i].open){
			printf("Unmount not possible. There are open files\n");
			return -1;
		}
	}
	writeMetadataToDisk(); 
	mounted = 0;
	return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)////////////////////////////////FALTA -1
{
	int b_id;	//primer bloque de datos del fichero
	int inodo_id;	//inodo asociado al fichero
	
	if(namei(fileName)>0){
		printf("File already exists\n");
		return -1;
	}
	
	inodo_id = ialloc();	//asigna el primer inodo libre
	if (inodo_id < 0){
		printf("Not possible. Max number of files already reached\n");
		return -2;
	}
	b_id = alloc();		//asigna el primer bloque de datos libre
	if(b_id < 0){
		ifree(inodo_id);
		printf("Not possible. No space in disk for a new file\n");
		return -2;
	}
	inodos[inodo_id].type = TIPO_FILE; // tipo 1 fichero
	strcpy(inodos[inodo_id].name, fileName); //escribimos el nombre
	inodos[inodo_id].num_block[0] = b_id ; 
	for(int i=1; i < 5;i++) inodos[inodo_id].num_block[i]=-2; //asignamos -2  
	inodos[inodo_id].size = 0;
	inodos[inodo_id].CRC = 0;
	inodox[inodo_id].position = 0;
	inodox[inodo_id].open = 1;
	return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	int inodo_id;

	inodo_id = namei(fileName);
	if(inodo_id < 0){
		printf("File does not exist\n");
		return -1;
	}
	if(inodox[inodo_id].open==1){
		printf("File is opened, not possible to remove\n");
		return -2;
	}
	for(int i=0; i<MAX_BI; i++){
		int bloque = inodos[inodo_id].num_block[i];
		if(bloque >= 0) bfree(bloque);
	}

	memset(&(inodos[inodo_id]), 0, sizeof(inode));
	ifree(inodo_id);

	return 0;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	int inodo_id;
 	inodo_id = namei(fileName);

 	if (inodo_id < 0){
		printf("File does not exist\n");
		return -1;
	}

	if (inodox[inodo_id].open==1){
		printf("File is already opened\n");
		return -2;
	}
 	
	inodox[inodo_id].position = 0;
 	inodox[inodo_id].open = 1;

 	return inodo_id; 
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	if(fileDescriptor < 0){
		return -1;
	}

	inodox[fileDescriptor].position = 0;
	inodox[fileDescriptor].open = 0;

	return 0;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fd, void *buffer, int numBytes)
{
	if((fd<0) || (fd > MAX_FN)){
		return -1;
	}
	char b[BLOCK_SIZE];
	int b_id;
	if(numBytes <= 0){
		printf("bytes to read must be positive\n");
		return -1;
	}
	if(inodox[fd].position + numBytes > inodos[fd].size){
		numBytes = inodos[fd].size - inodox[fd].position;
	}
	if(numBytes == 0) {
		printf("eof");
		return 0; //EOF
	}
	/*if(numBytes<0){	lseek no permite fuera de los limites
		printf("error, position > size\n");
		return -1;
	}*/
	int startPosition = inodox[fd].position % BLOCK_SIZE;
	int offsetbuffer = 0;
	
	while((startPosition + numBytes) > BLOCK_SIZE){
		b_id = bmap(fd, inodox[fd].position);
		bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
		memmove(buffer + offsetbuffer, b+ startPosition, BLOCK_SIZE - startPosition);
		inodox[fd].position += (BLOCK_SIZE - startPosition);
		numBytes -= BLOCK_SIZE - startPosition;
		offsetbuffer += BLOCK_SIZE - startPosition; 
		startPosition = 0;
	}
	b_id = bmap(fd, inodox[fd].position);
	bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
 	memmove(buffer + offsetbuffer, b + startPosition, numBytes);
	inodox[fd].position += numBytes;
	offsetbuffer += numBytes; 
	return offsetbuffer;
}
 
/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fd, void *buffer, int numBytes)
{
	if((fd<0) || (fd > MAX_FN)){
		return -1;
	}
	char b[BLOCK_SIZE];
	int b_id;
	if(numBytes <= 0){
		printf("bytes to write must be positive\n");
		return -1;
	}
	if(inodox[fd].position + numBytes > MAX_FS){
		numBytes = MAX_FS - inodox[fd].position;
	}
	if(numBytes == 0) {
		printf("Nothing to write");	//añadir en la memoria caso para numBytes introducidos = 0
		return 0; 
	}
	/*if(numBytes<0){	lseek no permite fuera de los limites
		printf("error, position > size\n");
		return -1;
	}*/
	int startPosition = inodox[fd].position % BLOCK_SIZE;
	int offsetbuffer = 0;
	
	while((startPosition + numBytes) > BLOCK_SIZE){
		b_id = bmap(fd, inodox[fd].position);
		if(b_id == -2){//si el bloque no estña asignado
			b_id = alloc();
			if(b_id == -1){return -1;}//si el no se puede reservar bloque
			inodos[fd].num_block[inodox[fd].position / BLOCK_SIZE] = b_id;//calculamos bloque correspondiente
		}
		bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
		memmove(b+ startPosition, buffer + offsetbuffer, BLOCK_SIZE - startPosition);
		bwrite(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
		inodox[fd].position += (BLOCK_SIZE - startPosition);
		numBytes -= BLOCK_SIZE - startPosition;
		offsetbuffer += BLOCK_SIZE - startPosition; 
		startPosition = 0;
	}
	b_id = bmap(fd, inodox[fd].position);
	if(b_id == -2){//si el bloque no estña asignado
		b_id = alloc();
		if(b_id == -1){return -1;}//si el no se puede reservar bloque
		inodos[fd].num_block[inodox[fd].position / BLOCK_SIZE] = b_id;//calculamos bloque correspondiente
	}
	bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
 	memmove(b + startPosition, buffer + offsetbuffer, numBytes);
	bwrite(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
	inodox[fd].position += numBytes;
	offsetbuffer += numBytes; 
	return offsetbuffer;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fd, long offset, int whence)
{	
	//comprobacion de errores
	if((fd<0) || (fd > MAX_FN)){
		return -1;
	}
	if(offset>inodos[fd].size || offset<0){
		return -1;	
	}

	//cambio de posiciones
	switch (whence){
		case FS_SEEK_CUR: inodox[fd].position = offset;
		     break;
		case FS_SEEK_BEGIN: inodox[fd].position = 0;
		     break;
		case FS_SEEK_END: inodox[fd].position = inodos[fd].size;
		     break;
	}
	
	return 0;
}

/*
 * @brief	Checks the integrity of the file.
 * @return	0 if success, -1 if the file is corrupted, -2 in case of error.
 */

int checkFile (char * fileName)
{
    return -2;
}

/*
 * @brief	Include integrity on a file.
 * @return	0 if success, -1 if the file does not exists, -2 in case of error.
 */

int includeIntegrity (char * fileName)
{
    return -2;
}

/*
 * @brief	Opens an existing file and checks its integrity
 * @return	The file descriptor if possible, -1 if file does not exist, -2 if the file is corrupted, -3 in case of error
 */
int openFileIntegrity(char *fileName)
{

    return -2;
}

/*
 * @brief	Closes a file and updates its integrity.
 * @return	0 if success, -1 otherwise.
 */
int closeFileIntegrity(int fileDescriptor)
{
    return -1;
}

/*
 * @brief	Creates a symbolic link to an existing file in the file system.
 * @return	0 if success, -1 if file does not exist, -2 in case of error.
 */
int createLn(char *fileName, char *linkName)
{
    return -1;
}

/*
 * @brief 	Deletes an existing symbolic link
 * @return 	0 if the file is correct, -1 if the symbolic link does not exist, -2 in case of error.
 */
int removeLn(char *linkName)
{
    return -2;
}
