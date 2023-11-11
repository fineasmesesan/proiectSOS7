#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#define BUF_SIZE 1024

typedef struct {
    uint16_t tip;
    uint32_t dimensiune;
    uint16_t rezervat1;
    uint16_t rezervat2;
    uint32_t offset;
    uint32_t dimensiune_header;
    int32_t latime;
    int32_t inaltime;
    uint16_t planuri;
    uint16_t biti_pe_pixel;
    uint32_t compresie;
    uint32_t dimensiune_imagine;
    int32_t x_pixeli_pe_metru;
    int32_t y_pixeli_pe_metru;
    uint32_t culori_totale;
    uint32_t culori_importante;
} AntetBMP;

void proceseazaElement(const char *numeElement, const char *caleDirector);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(STDERR_FILENO, "Utilizare: ./program <director_intrare>\n", 40);
        return 1;
    }

    proceseazaElement(argv[1], NULL);

    return 0;
}

void proceseazaElement(const char *numeElement, const char *caleDirector) {
    char caleElement[BUF_SIZE];
    if (caleDirector == NULL) {
        strcpy(caleElement, numeElement);
    } else {
        snprintf(caleElement, sizeof(caleElement), "%s/%s", caleDirector, numeElement);
    }

    struct stat infoElement;
    if (lstat(caleElement, &infoElement) == -1) {
        perror("Eroare obtinere informatii element");
        return;
    }

    if (S_ISREG(infoElement.st_mode)) {
        if (strstr(numeElement, ".bmp") != NULL) {
            int fisierBMP = open(caleElement, O_RDONLY);
            if (fisierBMP == -1) {
                perror("Eroare deschidere fisier BMP");
                return;
            }

            AntetBMP antet;
            if (read(fisierBMP, &antet, sizeof(AntetBMP)) != sizeof(AntetBMP)) {
                perror("Eroare citire antet BMP");
                close(fisierBMP);
                return;
            }

            close(fisierBMP);

            int fisierStatistici = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (fisierStatistici == -1) {
                perror("Eroare creare fisier de statistici");
                return;
            }

            time_t timpModificare = antet.dimensiune_imagine;

            char statistici[BUF_SIZE];
            sprintf(statistici, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %u\n"
                                "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %s"
                                "\ncontorul de legaturi: %dl\ndrepturi de acces user: %o\ndrepturi de acces grup: %o"
                                "\ndrepturi de acces altii: %o\n",
                    numeElement, antet.inaltime, antet.latime, antet.dimensiune, antet.dimensiune_header,
                    ctime(&timpModificare), antet.culori_totale,
                    antet.x_pixeli_pe_metru, antet.y_pixeli_pe_metru, antet.culori_importante);

            if (write(fisierStatistici, statistici, strlen(statistici)) == -1) {
                perror("Eroare scriere in fisier de statistici");
            }

            close(fisierStatistici);
        } else {
            int fisierStatistici = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (fisierStatistici == -1) {
                perror("Eroare creare fisier de statistici");
                return;
            }

            char statistici[BUF_SIZE];
            sprintf(statistici, "nume fisier: %s\n"
                                "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %s"
                                "\ncontorul de legaturi: %ld\ndrepturi de acces user: %o\ndrepturi de acces grup: %o"
                                "\ndrepturi de acces altii: %o\n",
                    numeElement, infoElement.st_uid, ctime(&infoElement.st_mtime),
                    infoElement.st_nlink, infoElement.st_mode & 0700,
                    (infoElement.st_mode & 0070) >> 3, infoElement.st_mode & 0007);

            if (write(fisierStatistici, statistici, strlen(statistici)) == -1) {
                perror("Eroare scriere in fisier de statistici");
            }

            close(fisierStatistici);
        }
    } else if (S_ISLNK(infoElement.st_mode)) {
        char dest[BUF_SIZE];
        ssize_t len = readlink(caleElement, dest, sizeof(dest)-1);
        if (len != -1) {
            dest[len] = '\0';

            int fisierStatistici = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (fisierStatistici == -1) {
                perror("Eroare creare fisier de statistici");
                return;
            }

            char statistici[BUF_SIZE];
            sprintf(statistici, "nume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\n"
                                "drepturi de acces user legatura: %o\ndrepturi de acces grup legatura: %o"
                                "\ndrepturi de acces altii legatura: %o\n",
                    numeElement, infoElement.st_size, infoElement.st_size,
                    infoElement.st_mode & 0700, (infoElement.st_mode & 0070) >> 3, infoElement.st_mode & 0007);

            if (write(fisierStatistici, statistici, strlen(statistici)) == -1) {
                perror("Eroare scriere in fisier de statistici");
            }

            close(fisierStatistici);
        } else {
            perror("Eroare citire legatura simbolica");
        }
    } else if (S_ISDIR(infoElement.st_mode)) {
        int fisierStatistici = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        if (fisierStatistici == -1) {
            perror("Eroare creare fisier de statistici");
            return;
        }

        char statistici[BUF_SIZE];
        sprintf(statistici, "nume director: %s\nidentificatorul utilizatorului: %d\n"
                            "drepturi de acces user: %o\ndrepturi de acces grup: %o"
                            "\ndrepturi de acces altii: %o\n",
                numeElement, infoElement.st_uid, infoElement.st_mode & 0700,
                (infoElement.st_mode & 0070) >> 3, infoElement.st_mode & 0007);

        if (write(fisierStatistici, statistici, strlen(statistici)) == -1) {
            perror("Eroare scriere in fisier de statistici");
        }

        close(fisierStatistici);

        DIR *dir = opendir(caleElement);
        if (dir == NULL) {
            perror("Eroare deschidere director");
            return;
        }

        struct dirent *dp;
        while ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                proceseazaElement(dp->d_name, caleElement);
            }
        }

        closedir(dir);
    }
}
