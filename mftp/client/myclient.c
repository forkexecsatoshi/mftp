#include "myclient.h"



int string_split(char* str, char del, int* countp, char*** vecp)
{
	char** vec;
	int  count_max, i, len;
	char* s, * p;

	if (str == 0)
		return(-1);
	count_max = countchr(str, del) + 1;
	vec = malloc(sizeof(char*) * (count_max + 1));
	if (vec == 0)
		return(-1);

	for (i = 0; i < count_max; i++)
	{
		while (*str == del)
			str++;
		if (*str == 0)
			break;
		for (p = str; *p != del && *p != 0; p++)
			continue;
		/* *p == del || *p=='\0' */
		len = p - str;
		s = malloc(len + 1);
		if (s == 0)
		{
			int j;
			for (j = 0; j < i; j++)
			{
				free(vec[j]);
				vec[j] = 0;
			}
			return(-1);
		}
		memcpy(s, str, len);
		s[len] = 0;
		vec[i] = s;
		str = p;
	}
	vec[i] = 0;
	*countp = i;
	*vecp = vec;
	return(i);
}

void free_string_vector(int qc, char** vec)
{
	int i;
	for (i = 0; i < qc; i++)
	{
		if (vec[i] == NULL)
			break;
		free(vec[i]);
	}
	free(vec);
}

int countchr(char* s, char c)
{
	int count;
	for (count = 0; *s; s++)
		if (*s == c)
			count++;
	return(count);
}
