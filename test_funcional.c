
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	01/03/2017
 *
 */


#include <stdio.h>
#include <string.h>
#include "filesystem/filesystem.h"
#include "filesystem/metadata.h"

// Color definitions for asserts
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BLUE "\x1b[34m"

#define N_BLOCKS 300					  // Number of blocks in the device
#define DEV_SIZE N_BLOCKS *BLOCK_SIZE // Device size, in bytes

int main()
{
	int ret;
	int fd;
	
	///////
	ret = mkFS(DEV_SIZE);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = mountFS();
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = createFile("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	fd = ret;
	///////

	char buffer[MAX_FILE_SIZE];
	//char *linea = "Esto es una linea de prueba para rellenar ficheros.\n";
	//strcpy(buffer, linea);
	for (int i = 0; i < MAX_FILE_SIZE; i++) buffer[i] = 'x'; //strcat(buffer, linea);
	
	ret = openFile("/test.txt");
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	ret = writeFile(fd, buffer, strlen(buffer));
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	fprintf(stdout, "Escritos %d bytes.\n", ret);
	
	///////

	ret = closeFile(fd);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	////////////////
	
	ret = openFile("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	fd = ret;

	///////
	
	ret = lseekFile(fd, 0, FS_SEEK_END);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	///////
	
	ret = lseekFile(fd, -10, FS_SEEK_CUR);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	///////
	
	ret = lseekFile(fd, -10, FS_SEEK_BEGIN);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	////////////////

	memset(buffer, 0, MAX_FILE_SIZE);
	ret = readFile(fd, buffer, MAX_FILE_SIZE);
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	fprintf(stdout, "Leidos %d bytes.\n", ret);
	char buff[1000];
	strncpy(buff, buffer, 999);
	fprintf(stdout, "%s\n", buff);

	///////

	ret = closeFile(fd);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	//////////

	ret = includeIntegrity("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST includeIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST includeIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	//////////

	ret = checkFile("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	//////////

	ret = openFileIntegrity("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	fd = ret;

	//////////

	ret = writeFile(fd, "Prueba" , 6);
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	//////////
	
	ret = closeFileIntegrity(fd);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	//////////

	ret = createLn("/test.txt", "/enlace.lnk");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createLn ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createLn ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	//////////
/*
	fd = createFile("Prueba.txt");
	printf("%d\n", fd);
	if (fd < 0) return -1;
	char buffer1[MAX_FN] = "Gilipollas";
	ret = writeFile(fd, buffer1, strlen(buffer1));
	printf("%d\n", ret);
	if (ret < 0) return -1;
	ret = closeFile(fd);
	printf("%d\n", ret);
	if (ret != 0) return -1;
	ret = createLn("Prueba.txt", "enlace2.lnk");
	printf("%d\n", ret);
	if (ret != 0) return -1;
	fd = openFile("enlace2.lnk");
	if (fd < 0) return -1;
	printf("%d", fd);
	ret = closeFile(fd);
	printf("%d\n", ret);
	if (ret != 0) return -1;
	ret = removeFile("enlace2.lnk");
	printf("%d\n", ret);
	if (ret != 0) return -1;
*/	
	//////////
	ret = removeLn("/enlace.lnk");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeLn ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeLn ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	
	/////////

	ret = unmountFS();
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	return 0;
}
