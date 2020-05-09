
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

// Variables globales
super_block superbloque; 
inode inodos[MAX_FN]; 
inode_x inodox[MAX_FN]; 
int mounted = 0; // boolean que indica si el sistema de archivos está montado


/*
 * @brief Aux function which writes superblock and inodes from memory to disk.
 */
void writeMetadataToDisk() {
	char bloque0[BLOCK_SIZE];
	char bloque1[BLOCK_SIZE];

	// bloque 0 Meto superbloque al principio del bloque 0
	memset(bloque0, 0, BLOCK_SIZE);
	memmove(bloque0, (char *)&superbloque, sizeof(superbloque));

	// bloque 0 Meto 16 inodos a continuacion 16 * 64B = 1KB
	memmove(bloque0 + sizeof(superbloque), (char *)inodos, 16 * sizeof(inode));

	// escribo bloque0 a disco
	bwrite(DEVICE_IMAGE, 0, bloque0);
	
	// bloque 1 Meto 32 inodos (del 16 al 47) 32 * 64B = 2KB
	memset(bloque1, 0, BLOCK_SIZE);
	memmove(bloque1, (char *)&(inodos[16]), 32 * sizeof(inode));

	// escribo bloque1 a disco
	bwrite(DEVICE_IMAGE, 1, bloque1);
}

/*
 * @brief 	Aux function which reads superblock and inodes from disk to memory.
 */
void readMetadataFromDisk() {
	char bloque0[BLOCK_SIZE];
	char bloque1[BLOCK_SIZE];
	
	// borro buffers
	memset(bloque0, 0, BLOCK_SIZE);
	memset(bloque1, 0, BLOCK_SIZE);

	// leer principio de bloque 0 de disco a superbloque
	bread(DEVICE_IMAGE, 0, bloque0);
	memmove((char *)&superbloque, bloque0, sizeof(superbloque));
	
	// A continuacion leo los primeros 16 inodos de bloque 0  16 * 64B = 1KB
	memmove((char *)&inodos, bloque0 + sizeof(superbloque), 16 * sizeof(inode));

	// Leo los 32 siguientes inodos de bloque 1  32 * 64B = 2KB
	bread(DEVICE_IMAGE, 1, bloque1);
	memmove((char *)&(inodos[16]), bloque1, 32 * sizeof(inode));
}

/*
 * @brief 	Aux function which looks for a free inode.
 * @return 	0 if success, -1 otherwise.
 */
int ialloc() {
	
	// buscar un i-nodo libre
	for (int i = 0; i < MAX_FN; i++)
	{
		if (superbloque.inode_map[i] == 0) { // si el inodo esta libre
			// marco inodo como ocupado
			superbloque.inode_map[i] = 1;
			// escribo valores por defecto en el i-nodo (borro todo a 0)
			memset(&(inodos[i]), 0, sizeof( inode));
			// devolver identificador de i-nodo
			return i;
		}
	}
	return -1; // no hay hueco para mas inodos
}

/*
 * @brief 	Aux function which looks for a free block.
 * @return 	block number if success, -1 otherwise.
 */
int alloc() {
	char b[BLOCK_SIZE];
	int numBloquesDatos = superbloque.numBlocks - 2;	//tamaño de disco (bloques) - 2 bloques de metadatos

	for (int i = 0; i < numBloquesDatos; i++) {
		if (superbloque.block_map[i] == 0) { // si el bloque esta libre
			
			// marco bloque como ocupado
			superbloque.block_map[i] = 1;
			
			// escribo valores por defecto en el bloque (lo borro todo a 0)
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
void ifree(int inodo) {
	
	// simplemente marco inodo como libre
	superbloque.inode_map[inodo] = 0; 
}

/*
 * @brief 	Aux function which frees a data block
 */
void bfree(int bloque) {
	
	// marco bloque como libre
	superbloque.block_map[bloque] = 0;
}


/*
 * @brief 	Aux function which returns inode_id from a filename
 * @return inode_id if success, -1 otherwise.
 */
int namei(char *filename) {

	for(int i = 0; i < MAX_FN; i++){ // busco en el array de inodos
		if(strcmp(inodos[i].name, filename) == 0){  // si encuentro el fichero que busco
			
			// para enlaces devuelvo fd del file enlazado original
			if (inodos[i].type == TIPO_LINK) return inodos[i].link;
			
			// para files normales devuelvo id
			return i;
		}
	}
	return -1; // no encuentra filename
}

/*
 * @brief 	Aux function which gives block number containig a certain position in a file
 * If the position is bigger than file size then allocs a new data block and return its number
 * @return  block number, -1 if error
 */
int bmap(int fd, int pos) {

	if(pos > MAX_FILE_SIZE) {
		// printf("Position overpass file size limit\n");
		return -1; // error, posicion mayor que el limite de tamaño
	}
	
	// calculo en cual de los 5 bloques posibles estoy y obtengo el num de bloque 
	int result = inodos[fd].num_block[pos / BLOCK_SIZE];

	// si la posicion es de un bloque sin asignar, lo asigno y obtengo su num de bloque
	if (result == -2) result = alloc();

	// Devuelvo el num de bloque o -1 en el caso de que alloc no pueda asignar mas bloques
	return result;
}

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize) {

	if (deviceSize < MIN_DS || deviceSize > MAX_DS){
		//printf("Disk size %ldKB not valid (460KB - 600KB)\n", deviceSize / 1024); // añadido
		return -1; // error, tamaño de disco fuera de especificaiones 
	}

	// rellenamos superbloque con valores por defecto y segun tamaño de disco
	superbloque.magic_num = 12345;
	superbloque.numBlocks = deviceSize / BLOCK_SIZE;
	if (superbloque.numBlocks > MAX_UB + 2) superbloque.numBlocks = MAX_UB + 2; // 2blocks para super+inodos
	
	for(int i = 0; i < MAX_FN; i++){
		superbloque.inode_map[i] = 0;
	} 
	
	for(int i = 0; i < MAX_UB; i++){
		superbloque.block_map[i] = 0;
	}
	
	// reseteamos todos los inodos (los borro con 0s)
	for(int i = 0; i< MAX_FN; i++){
		memset(&(inodos[i]), 0, sizeof(inode)); 
	}

	// Inicializamos los inodox (Todos los ficheros cerrados)
	for(int i = 0; i< MAX_FN; i++) {
		inodox[i].open = 0;
		inodox[i].position = 0;
	}

	// escribimos los valores por defecto al disco
	writeMetadataToDisk();  

	return 0;
}


/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void) {

	if(mounted == 1) {
		//printf("File system is already mounted\n");
		return -1; // error, el disco ya estaba montado
	}

	// leemos superbloque e inodos de disco
	readMetadataFromDisk();

	// marcamos como montado
	mounted = 1;

	return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void) {
	
	// comprobamos que no haya ficheros abiertos
	for(int i = 0; i < MAX_FN; i++){ 
		if(0 != inodox[i].open){
			//printf("Unmount not possible. There are open files\n");
			return -1; // error, fichero abierto
		}
	}

	// escribimos superbloque e inodos a disco
	writeMetadataToDisk();

	// marcamos como no montado
	mounted = 0;
	return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName) {
	int b_id;	//primer bloque de datos del fichero
	int inodo_id;	//inodo asociado al fichero
	
	//comprobamos que el nombre del archivo tenga máximo 32 caracteres más el char 0 al final, en total 33B
	if(strlen(fileName) > 32 || strlen(fileName) < 1){
		//printf("File's name must have 33B maximum including char 0 at the end, and 1 char at minimum\n");
		//printf("Size of name: %ld\nLenght of name:%ld\n", sizeof(fileName), strlen(fileName));
		return -1;
	}

	// comprobamos si ya existe un file con ese nombre
	if(namei(fileName) > 0){
		//printf("File already exists\n");
		return -1; // error,el fichero ya existe
	}
	
	// obtenemos el primer inodo libre
	inodo_id = ialloc();
	if (inodo_id < 0){
		//printf("Not possible. Max number of files already reached\n");
		return -2; // error, no hay inodos disponibles
	}

	// obtenemos el primer bloque de datos libre
	b_id = alloc();		
	if(b_id < 0){
		ifree(inodo_id);
		//printf("Not possible. No space in disk for a new file\n");
		return -2; // error, no hay bloques de datos disponibles
	}

	// rellenamos valores del nuevo inodo
	inodos[inodo_id].type = TIPO_FILE; // tipo 1 fichero
	strcpy(inodos[inodo_id].name, fileName); //escribimos el nombre
	inodos[inodo_id].num_block[0] = b_id ; // asignamos el primer bloque de datos
	for(int i=1; i < 5;i++) inodos[inodo_id].num_block[i] = -2; // -2 -> (bloques no asignados)
	inodos[inodo_id].size = 0; // tamaño inicial 0
	inodos[inodo_id].CRC = 0; // 0 -> no CRC info
	inodos[inodo_id].link = -1 ; // -1 -> no enlazado

	// rellenamos valores del inodox
	inodox[inodo_id].position = 0; // posicion inicial
	//inodox[inodo_id].open = 1; // marcamos file como abierto ?? DUDA preguntar -> NO, se queda cerrado

	return 0; // ok
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName) {
	int inodo_id;

	// comprobamos si existe
	inodo_id = namei(fileName);
	if(inodo_id < 0){
		// printf("File does not exist\n");
		return -1; // el file no existe
	}

	// comprobamos que no este abierto
	if(inodox[inodo_id].open != 0){
		// printf("File is opened, not possible to remove\n");
		return -2; // error, fichero abierto
	}

	// liberamos los bloques de datos
	for(int i = 0; i < MAX_BI; i++){
		int bloque = inodos[inodo_id].num_block[i];
		if(bloque >= 0) bfree(bloque); // solo liberamos los que esten asignados (-2 -> no asignado)
	}

	// Si el fichero tiene enlaces borramos todos los enlaces
	int numLinks = inodos[inodo_id].link;
	for (int i = 0; i < MAX_FN; i++) { // busco en el array de inodos
		if (inodos[i].type == TIPO_LINK && inodos[i].link == inodo_id) { // si es un enlace que apunta a file
			memset(&(inodos[i]), 0, sizeof(inode)); // borro el inodo del enlace
			ifree(i); // libero el inodo
			numLinks--; // decremento el num de enlaces a file. Si llego a 0 termino
			if (numLinks == 0) break;
		}
	}

	// Borramos y liberamos el inodo del file
	memset(&(inodos[inodo_id]), 0, sizeof(inode));
	ifree(inodo_id);

	return 0; // ok
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName) {
	int inodo_id;

	// buscamos el fd del file y comprobamos si existe
 	inodo_id = namei(fileName);
 	if (inodo_id < 0){
		printf("File does not exist\n");
		return -1; // file no existe
	}
	
	// comprobamos que no este abierto
	if (inodox[inodo_id].open != 0){
		printf("File is already opened\n");
		return -2;
	}
	
	// abrimos fichero y reseteamos posicion
	inodox[inodo_id].position = 0;
	inodox[inodo_id].open = 1;

	return inodo_id; 
}


/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor) {

	// comprobamos file descriptor correcto
	if(fileDescriptor < 0 || fileDescriptor > MAX_FN){
		//printf("Wrong file descriptor\n");
		return -1; // error, fd fuera de limites
	}

	//comprobamos si es un enlace, para que en tal caso, nos devuelva el descriptor del archivo
	int inodo_id = namei(inodos[fileDescriptor].name);

	// comprobamos file descriptor correcto
	if(fileDescriptor < 0 || fileDescriptor > MAX_FN){
		//printf("Wrong file descriptor\n");
		return -1; // error, fd fuera de limites
	}

	// comprobamos que no estuviese ya cerrado o que no este abierto con integridad
	if (inodox[inodo_id].open != 1){
		//printf("File is already closed\n");
		return -1; // error, ya estaba cerrado o estaba abierto con integridad
	} // files abiertos con integridad deben cerrarse con closeFileIntegrity() 
	
	inodox[inodo_id].position = 0;
	inodox[inodo_id].open = 0; // cerramos file

	return 0; // ok
}




/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fd, void *buffer, int numBytes) {
	char b[BLOCK_SIZE]; // buffer para datos leidos
	int b_id; // id del bloque de datos
	
	// comprobamos file descriptor correcto
	if(fd < 0 || fd > MAX_FN){
		 //printf("Wrong file descriptor\n");
		return -1; // error, fd fuera de limites
	}
	
	// comprobamos que no este cerrado
	if (inodox[fd].open == 0){
		 //printf("File is closed\n");
		return -1;
	}

	// comprobamos bytes a leer sean correctos
	if(numBytes < 0){
		//printf("bytes to read must be positive\n");
		return -1;
	}
	if(numBytes == 0){
		//printf("Nothing to read\n");
		return 0;
	}

	// limitamos para que no se pueda leer mas alla del fin del fichero
	if(inodox[fd].position + numBytes > inodos[fd].size){
		numBytes = inodos[fd].size - inodox[fd].position;
	}

	// devolvemos 0 si no se lee ningun byte.
	if(numBytes == 0) {
		//printf("eof\n");
		return 0; //EOF
	}

	/*if(numBytes<0){	lseek no permite fuera de los limites
		printf("error, position > size\n");
		return -1;
	}*/
	
	int startPosition = inodox[fd].position % BLOCK_SIZE; // posicion inicial dentro del primer bloque de datos
	int offsetbuffer = 0; // desplazamiento respecto el inicio de la salida
	int bytesToRead = numBytes; // bytes que quedan por leer

	// bucle mientras el num de bytes por leer nos lleve a otro bloque de datos
	while((startPosition + bytesToRead) > BLOCK_SIZE){
		b_id = bmap(fd, inodox[fd].position); // obtenemos el bloque de datos
		bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b); // lo leemos de disco al buffer b
		// leemos del buffer b a la salida
		memmove(buffer + offsetbuffer, b + startPosition, BLOCK_SIZE - startPosition);
		inodox[fd].position += (BLOCK_SIZE - startPosition); // actualizamos posicion
		bytesToRead -= (BLOCK_SIZE - startPosition); // actualizamos los bytes por leer
		offsetbuffer += (BLOCK_SIZE - startPosition); // actualizamos offset
		startPosition = 0; // en los siguientes bloque la posicion inicial es 0
	}
	// leemos el ultimo bloque de datos (puede que sea el unico)
	b_id = bmap(fd, inodox[fd].position);
	bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
 	memmove(buffer + offsetbuffer, b + startPosition, bytesToRead);
	
	// actualizamos posicion
	inodox[fd].position += numBytes;

	// devolvemos el num de bytes leidos
	return numBytes;
}
 
/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fd, void *buffer, int numBytes) {
	char b[BLOCK_SIZE]; // buffer para datos leidos
	int b_id; // id del bloque de datos
	
	// comprobamos file descriptor correcto
	if(fd < 0 || fd > MAX_FN){
		 //printf("Wrong file descriptor\n");
		return -1; // error, fd fuera de limites
	}
	
	// comprobamos que no este cerrado
	if (inodox[fd].open == 0){
		 //printf("File is closed\n");
		return -1;
	}
	
	// comprobamos bytes a escribr sean correctos
	if(numBytes < 0){
		//printf("bytes to write must be positive\n");
		return -1;
	}

	// limitamos para que no se pueda escribir mas alla del tamaño maximo de fichero
	if(inodox[fd].position + numBytes > MAX_FILE_SIZE){
		numBytes = MAX_FILE_SIZE - inodox[fd].position;
	}

	// devolvemos 0 si no se escribe ningun byte.
	if(numBytes == 0) {
		//printf("eof\n");
		return 0; //EOF
	}

	/*if(numBytes<0){	lseek no permite fuera de los limites
		printf("error, position > size\n");
		return -1;
	}*/
	//printf("Comprobaciones hechas, vamos a escribir\n");

	int startPosition = inodox[fd].position % BLOCK_SIZE; // posicion inicial dentro del primer bloque de datos
	int offsetbuffer = 0; // desplazamiento respecto el inicio de la entrada
	int bytesToWrite = numBytes; // bytes que quedan por escribir
	
	// bucle mientras el num de bytes por escribir nos lleve a otro bloque de datos
	while((startPosition + bytesToWrite) > BLOCK_SIZE){
		// obtenemos el bloque de datos de la posicion o uno nuevo si hace falta
		b_id = bmap(fd, inodox[fd].position);
		if(b_id == -1) {
			 //printf("Not enough disk space\n"); // no hay espacio en disco y por lo tanto da error
			return -1; // error, no se ha podido asignar un bloque de datos nuevo.
		}
		// leemos el bloque de disco al buffer b
		bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
		// rellenamos con la entrada desde posicion inicial
		memmove(b+ startPosition, buffer + offsetbuffer, BLOCK_SIZE - startPosition);
		// escribimos buffer b a disco
		bwrite(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b);
		inodox[fd].position += (BLOCK_SIZE - startPosition); // actualizamos posicion
		bytesToWrite -= (BLOCK_SIZE - startPosition); // actualizamos bytes por escribir
		offsetbuffer += (BLOCK_SIZE - startPosition); // actualizamos offset
		startPosition = 0; // en los siguientes bloque la posicion inicial es 0
	}
	// escribimos el ultimo bloque de datos (puede que sea el unico)
	b_id = bmap(fd, inodox[fd].position);// bloque de datos de la posicion o uno nuevo si hace falta
	if(b_id == -1) {
		//printf("Not enough space in disk\n"); // no hay espacio en disco y por lo tanto da error
		return -1; // error, no se ha podido asignar un bloque de datos nuevo.
	}
	bread(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b); // leemos el bloque de disco al buffer b
	memmove(b+ startPosition, buffer + offsetbuffer, bytesToWrite); // rellenamos con la entrada
	bwrite(DEVICE_IMAGE, FIRST_DATA_BLOCK + b_id, b); // escribimos buffer b a disco
		
	// actualizamos posicion
	inodox[fd].position += bytesToWrite;
	
	// actualizamos tamaño del file
	if (inodox[fd].position > inodos[fd].size) inodos[fd].size = inodox[fd].position;
	
	// devuelve bytes escritos
	return numBytes;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fd, long offset, int whence) {	
	
	// comprobamos file descriptor correcto
	if(fd < 0 || fd > MAX_FN){
		// printf("Wrong file descriptor\n");
		return -1; // error, fd fuera de limites
	}

	// comprobamos que no este cerrado
	if (inodox[fd].open == 0){
		// printf("File is closed\n");
		return -1;
	}
	
	// cambios de posiciones
	int newPosition;
	switch (whence){
		case FS_SEEK_CUR:
			newPosition = inodox[fd].position + offset; // posicion nueva
			if (newPosition >= 0 && newPosition <= inodos[fd].size) 
				inodox[fd].position = newPosition; // no puede pasar del final del file, ni antes del inicio
			else {
				// printf("Out of bounds. Offset not valid.\n");
				return -1; // error. intento de mover el cursor fuera de limites
			}
		    break;
		case FS_SEEK_BEGIN:
			inodox[fd].position = 0; // mover al principio
		    break;
		case FS_SEEK_END:
			inodox[fd].position = inodos[fd].size; // mover al final
		    break;
		default:
			// printf("Wrong seek option.\n");
			return -1; // error. opciones no contempladas
	}

	return 0; // ok
}

/*
 * @brief	Checks the integrity of the file.
 * @return	0 if success, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile (char * fileName) {
	unsigned char buffer[MAX_FILE_SIZE];
	
	// comprobamos que el file exista
	int fd = namei(fileName);
	if (fd == -1) {
		// printf("File does not exist\n");
		return -2; // error, no existe el file
	}
	
	// comprobamos que el file no este abierto
	if (inodox[fd].open != 0) {
		//printf("File is opened\n");
		return -2; // error, el file esta abierto
	}

	// comprobamos que el file tenga integridad
	if (inodos[fd].CRC == 0) {
		// printf("File does not have integrity info\n");
		return -2;
	}

	// abrimos y leemos el file entero metiendolo en buffer
	fd = openFile(fileName);
	int size  = readFile(fd, buffer, inodos[fd].size);
	closeFile(fd);

	// calculamos integridad crc y comprobamos si coincide con la guardada en el inodo
	uint32_t crc = CRC32(buffer, size);
	if (crc != inodos[fd].CRC) {
		// printf("Integrity check failed. Corrupt file\n");
		return -1; // Se ha perdido la integridad. File corrupto
	}

	return 0; // ok
}

/*
 * @brief	Include integrity on a file.
 * @return	0 if success, -1 if the file does not exists, -2 in case of error.
 */
int includeIntegrity (char * fileName) {
    	unsigned char buffer[MAX_FILE_SIZE];
	
	// comprobamos que el file exista
    	int fd = namei(fileName);
	if (fd == -1) {
		//printf("File does not exist\n");
		return -1; // error, no existe el file
	}
	
	// comprobamos que el file no este abierto
	if (inodox[fd].open != 0) {
		//printf("File is opened\n");
		return -2; // error, el file esta abierto
	}

	// comprobamos que el file no tenga ya integridad
	if (inodos[fd].CRC != 0) {
		 //printf("File already has integrity info\n");
		return -2;
	}
	
	if(inodos[fd].size == 0){
		//printf("File empty, not possible to include integrity\n");
		return -1;
	}

	// abrimos y leemos el file entero metiendolo en buffer
	fd = openFile(fileName);
	//printf("File opened %d\n", fd);
	int size  = readFile(fd, buffer, inodos[fd].size);
	//printf("File read %d\n", size);
	closeFile(fd);
	//printf("File closed \n");
	// calculamos integridad crc y la guardamos en el inodo
	inodos[fd].CRC = CRC32(buffer, size);
	
	return 0; // ok
}

/*
 * @brief	Opens an existing file and checks its integrity
 * @return	The file descriptor if possible, -1 if file does not exist, -2 if the file is corrupted, -3 in case of error
 */
int openFileIntegrity(char *fileName) {
	
	// comprobamos que el file exista
	int fd = namei(fileName);
	if (fd == -1) {
		// printf("File does not exist\n");
		return -1; // error, no existe el file
	}

	// comprobamos integridad con checkfile.
	int result = checkFile(fileName);
	if (result == 0) { // si integridad ok abrimos el file en modo integridad
		inodox[fd].position = 0;
 		inodox[fd].open = 2; // abrimos
 		return fd; // ok, devolvemos fd del file
	}
	else if(result == -1) return -2; // checkfile da -1 -> file corrrupto. 

	// errores, checkfile da -2 y salimos con -3. 
	// Por ej. si file no tiene info de integridad o si el file estaba abierto
	return -3; 
}

/*
 * @brief	Closes a file and updates its integrity.
 * @return	0 if success, -1 otherwise.
 */
int closeFileIntegrity(int fileDescriptor) {
    
	// comprobamos file descriptor correcto
	if(fileDescriptor < 0 || fileDescriptor > MAX_FN){
		 //printf("Wrong file descriptor\n");
		return -1; // error, fd fuera de limites
	}

	// comprobamos que no estuviese ya cerrado o que no este abierto sin integridad
	if (inodox[fileDescriptor].open != 2){
		return -1; // error, ya estaba cerrado o estaba abierto sin integridad
	} // files abiertos sin integridad deben cerrarse con closeFile() 
	
	// reseteo info de integridad y la recalculo luego
	inodos[fileDescriptor].CRC = 0; 

	// reseteamos posicion y cerramos el file
	inodox[fileDescriptor].position = 0;
	inodox[fileDescriptor].open = 0;
	
	if (includeIntegrity(inodos[fileDescriptor].name) != 0) return -1; // error
	
	return 0; // ok
}


//////////////////////////////////////
// LINKS
// Supongo las siguientes especificaciones
// - Se pueden hacer varios links a un fichero
// - No se pueden hacer enlaces a enlaces
// - Create, open, close, read, write y seek de un link afectan al fichero original
// - removeFile(link) borra el fichero original y todos sus enlaces
// - removeLink(link) solo borra el enlace
/////////////////////////////////////

/*
 * @brief	Creates a symbolic link to an existing file in the file system.
 * @return	0 if success, -1 if file does not exist, -2 in case of error.
 */
int createLn(char *fileName, char *linkName) {
	int fd;	// inodo asociado al file
	int inodo_id;	//inodo asociado al link
	
	// comprobamos que el file existe
	fd = namei(fileName);
	if(fd < 0) {
		//printf("File does not exist\n");
		return -1;
	}

	// comprobamos que es de tipo file
	for (int i = 0; i < MAX_FN; i++){
		if(0 == strcmp(fileName, inodos[i].name)){
			inodo_id = i;
		}
	}

	if (inodos[inodo_id].type != TIPO_FILE){
		//printf("%s is a link, not a file\n" , fileName);
		return -2;
	}

	// comprobamos que el link no existe todavia
	if(namei(linkName) > 0){
		//printf("Link name already exists\n");
		return -2;
	}

	//asignamos al link el primer inodo libre
	inodo_id = ialloc();
	if (inodo_id < 0){
		//printf("Not possible. Max number of files already reached\n");
		return -2;
	}
	//printf("fd of link: %d\n", inodo_id);
	// rellenamos el nuevo inodo
	inodos[inodo_id].type = TIPO_LINK; // tipo 2 link
	strcpy(inodos[inodo_id].name, linkName); //escribimos el nombre del link
	for(int i = 0; i < 5; i++) inodos[inodo_id].num_block[i] = -2; // -2 -> (bloques no asignados)
	inodos[inodo_id].size = 0;
	inodos[inodo_id].CRC = 0; // 0 -> no CRC info
	inodos[inodo_id].link = fd; // enlazado a fd
	inodox[inodo_id].position = 0;
	//inodox[inodo_id].open = 0; -> No, se queda cerrado
	
	// aumentamos el numero de enlaces del file
	inodos[fd].link++; 

	return 0; // ok
}

/*
 * @brief 	Deletes an existing symbolic link
 * @return 	0 if the file is correct, -1 if the symbolic link does not exist, -2 in case of error.
 */
int removeLn(char *linkName) {
	int inodo_id = -1;
	
	// comprobamos que existe el link
	for(int i = 0; i < MAX_FN; i++){
		if(strcmp(inodos[i].name, linkName) == 0)	{
			if (inodos[i].type == TIPO_LINK) {
				inodo_id = i;
				break;
			}
			return -1; // link no existe (hay un file con su nombre)
		}
	}
	if (inodo_id == -1)	return -1; // link no existe
	
	// obtenemos file original y disminuimos su num de enlaces
	int fd = inodos[inodo_id].link;
	inodos[fd].link--;

	// borramos inodo del link
	memset(&(inodos[inodo_id]), 0, sizeof(inode)); // borramos y liberamos el inodo
	ifree(inodo_id);

	//liberamos el inodo x asociado
	memset(&(inodox[inodo_id]), 0, sizeof(inode_x));

	
	return 0; //ok
}

