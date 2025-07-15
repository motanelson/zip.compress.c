
/* zipwrap.c – Envolve vários ficheiros num ZIP sem compressão.
 * Compilação:  gcc -std=c99 -Wall -o zipwrap zipwrap.c -lzip
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
//sudo apt-get install libzip-dev 
//gcc -std=c99 -Wall -o zipwrap zipwrap.c -lzip
#define ZIP_NAME      "output.zip"
#define LINE_BUFSIZE  8192   /* tamanho máximo da linha lida */
#define ZIP_ERROR_BUFFER_SIZE 1024
int main(void)
{
    char line[LINE_BUFSIZE];
    printf("\033c\033[43;30m\n");
    puts("Introduza os ficheiros a incluir (separe por espaços):");
    if (!fgets(line, sizeof line, stdin)) {
        fputs("Erro a ler entrada.\n", stderr);
        return EXIT_FAILURE;
    }

    /* Abre/Trunca o ZIP */
    int err;
    int errno=0;                                                     
  /* para códigos da libzip */
    zip_t *zip = zip_open(ZIP_NAME, ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!zip) {
        char errbuf[ZIP_ERROR_BUFFER_SIZE];
        zip_error_to_str(errbuf, sizeof errbuf, err, errno);
        fprintf(stderr, "Não foi possível criar '%s': %s\n", ZIP_NAME, errbuf);
        return EXIT_FAILURE;
    }

    /* Tokeniza a linha por espaços */
    char *token = strtok(line, " \t\r\n");
    if (!token) {
        fputs("Nenhum ficheiro fornecido.\n", stderr);
        zip_discard(zip);
        return EXIT_FAILURE;
    }

    while (token) {
        /* Cria um zip_source a partir do próprio ficheiro */
        zip_source_t *src = zip_source_file(zip, token, 0, 0);   /* 0 bytes ⇒ tamanho total */
        if (!src) {
            fprintf(stderr, "Falhou abrir '%s' – ignorado.\n", token);
        } else {
            /* Adiciona com o nome‑base dentro do ZIP */
            const char *basename = strrchr(token, '/');
#ifdef _WIN32
            if (!basename) basename = strrchr(token, '\\');
#endif
            basename = basename ? basename + 1 : token;

            zip_int64_t idx = zip_file_add(zip, basename, src, ZIP_FL_OVERWRITE);
            if (idx < 0) {
                fprintf(stderr, "Erro a adicionar '%s' (%s)\n",
                        basename, zip_strerror(zip));
                zip_source_free(src);           /* tem de libertar caso falhe */
            } else {
                /* Força método STORE (sem compressão) */
                zip_set_file_compression(zip, idx, ZIP_CM_STORE, 0);
            }
        }
        token = strtok(NULL, " \t\r\n");
    }

    if (zip_close(zip) < 0) {
        fprintf(stderr, "Erro a fechar ZIP: %s\n", zip_strerror(zip));
        return EXIT_FAILURE;
    }

    puts("ZIP criado com sucesso: " ZIP_NAME);
    return EXIT_SUCCESS;
}
