#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

/* Parse text to JSON, then render back to text, and print! */
void doit(char *text, int num)
{
	char *out;cJSON *json;

	cJSON *node, *song;
	
	json=cJSON_Parse(text);
	if (!json) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		out=cJSON_Print(json);
		//printf("%s\n",out);

		node = cJSON_GetObjectItem(json, "songs");
		if(node)
		{
			int count = cJSON_GetArraySize(node);
			//printf("%d songs\n", count);
			song = cJSON_GetArrayItem(node, num - 1);
			printf("%s\n", cJSON_GetObjectItem(song, "path")->valuestring);
		}

		//cJSON_Delete(node);
		cJSON_Delete(json);
		free(out);
	}
}

/* Read a file, parse, render back, etc. */
void dofile(char *filename, int num)
{
	FILE *f;long len;char *data;
	
	f=fopen(filename,"rb");fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
	doit(data, num);
	free(data);
}

int main (int argc, const char * argv[]) {
	/* Parse standard testfiles: */
	dofile("song_edison.json", atoi(argv[1]));
/*	dofile("../../tests/test2"); */

	
	return 0;
}
