
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
#include "filesystem/auxiliary.h"

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

	char tooLong[33] = "/qwertyuiopasdfghjklzxcvbnmqwerty";
	char fine[33] = "/qwertyuiopasdfghjklzxcvbnmqwert";

	//Prueba 1: tamaño de disco demasiado pequeño, 471039  ///////////////////////////////////////////////////////////
	ret = mkFS(471039);
	if(ret == 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 1 mkFs tamaño demasiado pequeño", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 1 mkFs tamaño demasiado pequeño", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);


	//Prueba 2: tamaño de disco demasiado grande, 614401  ///////////////////////////////////////////////////////////
	ret = mkFS(614401);
	if(ret == 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 2 mkFs tamaño demasiado grande", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 2 mkFs tamaño demasiado grande", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 3: tamaño de disco mínimo, 471040  ////////////////////////////////////////////////////////////////////////////////////
	ret = mkFS(471040);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 4 mkFS tamaño mínimo", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 3 mkFS tamaño mínimo", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);


	//Prueba 4: tamaño de disco máximo, 614400 ///////////////////////////////////////////////////////////////////////////////
	ret = mkFS(614400);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 4 mkFS tamaño máximo", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 4 mkFS tamaño máximo", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 5: MOUNT DISK ////////////////////////////////////////////////////////////////////////////////////////
	ret = mountFS();
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 5 mountFS", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 5 mountFS", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);


	//Prueba 6: crear archivo de 33 caracteres ///////////////////////////////////////////////////////////////////////////
	ret = createFile(tooLong);
	if (ret == 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 6 createFile nombre demasiado grande", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 6 createFile nombre demasiado grande", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);


	//Prueba 7: crear archivo de 32 caracteres ///////////////////////////////////////////////////////////////////////////
	ret = createFile(fine);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 7 createFile nombre máximo", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 7 createFile nombre máximo", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);


	//Prueba 8: cerrar archivo ///////////////////////////////////////////////////////////////////////////
	ret = closeFile(ret);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 8 closeFile", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 8 closeFile", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);
	
	
	
	//Prueba 9: abrir archivo ///////////////////////////////////////////////////////////////////////////
	ret = openFile(fine);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 9 openFile", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 9 openFile", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 10: escribir en archivo ///////////////////////////////////////////////////////////////////////////
	fd = ret;
	char buffer[MAX_NL] = "hey motherfucker";
	ret = writeFile(ret, buffer, strlen(buffer));
	if (ret <= 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 10 writeFile", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 10 writeFile", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 11: cerrar archivo después de haberlo escrito///////////////////////////////////////////////////////////////////////////
	ret = closeFile(fd);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 11 closeFile after writing", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 11 closeFile writing", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);
	


	//Prueba 12: incluir integridad en archivo ///////////////////////////////////////////////////////////////////////////
	ret = includeIntegrity(fine);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 12 includeIntegrity", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 12 includeIntegrity", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);
	
	

	//Prueba 13: comprobar integridad de archivo ///////////////////////////////////////////////////////////////////////////
	ret = checkFile(fine);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 13 checkFile", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 13 checkFile", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 14: abrir archivo con integridad  ///////////////////////////////////////////////////////////////////////////
	ret = openFileIntegrity(fine);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 14 openFileIntegrity", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 14 openFileIntegrity", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);




	//Prueba 15: cerrar archivo con integridad  ///////////////////////////////////////////////////////////////////////////
	ret = closeFileIntegrity(ret);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 15 closeFileIntegrity", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST  15 closeFileIntegrity", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 16: crear enlace simbólico  ///////////////////////////////////////////////////////////////////////////
	ret = createLn(fine, "linkk");
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 16 createLn", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 16 createLn", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);


	//Prueba 17: abrir enlace simbólico ///////////////////////////////////////////////////////////////////////////
	ret = openFile("linkk");
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 17 abrir enlace simbólico", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 17 abrir enlace simbólico", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);

	
	
	//Prueba 18: umount disk cuando hay archivos abiertos  ///////////////////////////////////////////////////////////////////////////
	ret = unmountFS();
	if (ret == 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 18 unmountFS con archivos abiertos", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 18 unmountFS con archivos abiertos", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);
	



	int aux = namei("linkk");
	//Prueba 19: cerrar archivo por enlace simbólico///////////////////////////////////////////////////////////////////////////
	ret = closeFile(aux);
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 19 closeFile mediante enlace simbólico", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 19 closeFile mediante enlace simbólico", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 20: eliminar enlace simbólico  ///////////////////////////////////////////////////////////////////////////
	ret = removeLn("linkk");
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 20 removeLn", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 20 removeLn", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);



	//Prueba 21: umount disk ///////////////////////////////////////////////////////////////////////////
	ret = unmountFS();
	if (ret != 0){

		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 21 unmountFS", ANSI_COLOR_RED, " FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 21 unmountFS", ANSI_COLOR_GREEN, " SUCCESS\n", ANSI_COLOR_RESET);


	
	return 0;
}
