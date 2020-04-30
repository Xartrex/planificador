
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

struct super_block superbloque;
struct inode inodo[MAX_FN];
struct inode_x inodox[MAX_FN];
int mounted = 0;

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
	if (deviceSize < MIN_DS || deviceSize > MAX_DS){
		return -1;
	}
	superbloque.magic_num = 12345;
	superbloque.numBlocks = MAX_BI;

	for(int i = 0; i < MAX_FN; i++){
		superbloque.inode_map[i] = 0;
	} 
	for(int i = 0; i < superbloque.numBlocks; i++){
		superbloque.block_map[i] = 0;
	}
	for(int i = 0; i< MAX_FN; i++){
		memset(&(inodo[i]), 0, sizeof(inodo[i].type));
	}

	metadata_fromMemoryToDisk();

	return 0;
}


/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)///////////////////////DUDA SI SOLO BREAD O METADATA
{
	if(mounted == 1){
		return -1;
	}

	metadata_fromDiskToMemory(); 

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
		if(inodox[i].open = 1){
			return -1;
		}
	}

	metadata_fromDiskToMemory(); 

	mounted = 0;

	return -1;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)////////////////////////////////FALTA -1
{
	int b_id;
	int inodo_id;
	
	inodo_id = ialloc();
	if (inodo_id < 0){
		return -2;
	}

	b_id = alloc();
	if(b_id < 0 || b_id > 5){
		ifree(inodo_id);
		return -2;
	}

	inodo[inodo_id].type = 1;
	strcpy(inodo[inodo_id].name, fileName);
	inodos[inodo_id].numBlocks = b_id ;
	inodox[inodo_id].position = 0;
	inodox[inodo_id].open = 1;
	return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)//////////////////////////HAY QUE CONFIRMAR QUE SEA EL UNLINK Y COMPROBAR -1
{
	int inodo_id;

	inodo_id = namei(fileName);
	if(inodo_id < 0){
		return -2;
	}

	free(inodo[inodo_id].numBlocks);
	memset(&(inodo[inodo_id]), 0, sizeof(inodo[inodo_id].type));
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
		return -2;
	}

	if (inodo_id == NULL){//////////////////////////////////NO ESTAMOS SEGUROS DE QUE SEA ASI
		return -1;
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
	if(fd < 0){
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
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	char b[MAX_BS];
	int b_id;

	if(inodox[fileDescriptor].position + numBytes > inodo[fileDescriptor.size]){
		numBytes = inodo[fileDescriptor].size - inodox[fileDescriptor].position;
	}

	if(numBytes <= 0){
		return 0;
	}

	b_id = bmap(fileDescriptor, inodox[fileDescriptor].position);

	//bread(DISK, superbloque.primerBloqueDatos+b_id, b);
 	memmove(buffer, b+inodox[fileDescriptor].position, numBytes);
	inodox[fileDescriptor].position += numBytes;
	return numBytes;
}
 
/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
	return -1;
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
