/*
 * Monitoring système — version C / terminal (printf uniquement)
 *
 * Compilation :
 *   gcc monitoring.c -o monitoring -lm
 *
 * Fonctionne sur Linux (lit /proc/meminfo, /proc/stat, statvfs).
 * Quitter avec Ctrl+C.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>

/* ─────────────────────────────────────────────
   Constantes
   ───────────────────────────────────────────── */
#define SEUIL      80        /* seuil d'alerte en % */
#define BAR_WIDTH  30        /* largeur de la barre en caractères */
#define REFRESH_MS 800       /* intervalle de rafraîchissement (ms) */

/* Codes ANSI */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define CLEAR   "\033[2J\033[H"   /* efface l'écran + curseur en haut */

/* ─────────────────────────────────────────────
   Lecture des métriques via /proc
   ───────────────────────────────────────────── */

static double get_ram_percent(void) 
{

    FILE* f = fopen("/proc/meminfo", "r");
    char line[128];
    long total = 0, available = 0;

    if (!f) 
    {
        return 0.0;
    }
    while (fgets(line, sizeof line, f)) 
    {
        if (strncmp(line, "MemTotal:",     9)  == 0) 
        {
            sscanf(line, "MemTotal: %ld kB",     &total);
        }
        if (strncmp(line, "MemAvailable:", 13) == 0) 
        {
            sscanf(line, "MemAvailable: %ld kB", &available);
        }
    }
    fclose(f);
    if (total == 0)
    { 
        return 0.0;
    }
    return 100.0 * (total - available) / total;
}

typedef struct 
{ 
    long long idle, total; 
} CpuTimes;

static CpuTimes read_cpu_times(void)
{
    CpuTimes t = {0, 0};
    FILE* f = fopen("/proc/stat", "r");
    if (!f) return t;
    long long u, n, s, i, iow, irq, sirq;
    fscanf(f, "cpu %lld %lld %lld %lld %lld %lld %lld", &u, &n, &s, &i, &iow, &irq, &sirq);
    fclose(f);
    t.idle  = i + iow;
    t.total = u + n + s + i + iow + irq + sirq;
    return t;
}

static double get_cpu_percent(void) 
{
    CpuTimes t1 = read_cpu_times();
    usleep(150000);   /* 150 ms de mesure */
    CpuTimes t2 = read_cpu_times();
    long long dtotal = t2.total - t1.total;
    long long didle  = t2.idle  - t1.idle;
    if (dtotal == 0)
    {
        return 0.0;
    }
    return 100.0 * (dtotal - didle) / dtotal;
}

static double get_disk_percent(const char* path) 
{
    struct statvfs sv;
    if (statvfs(path, &sv) != 0) 
    {
        return 0.0;
    }
    double total = (double)sv.f_blocks * sv.f_frsize;
    double free_ = (double)sv.f_bfree  * sv.f_frsize;
    if (total == 0.0) 
    {
        return 0.0;
    }
    return 100.0 * (total - free_) / total;
}

/* ─────────────────────────────────────────────
   Affichage d'une barre en ASCII
   ───────────────────────────────────────────── */

/*
  Exemple de rendu pour 65% :
  RAM  [####################..........] 65.0%
*/
static void print_bar(const char* label, double val) 
{
    int filled = (int)(val * BAR_WIDTH / 100.0);
    if (filled > BAR_WIDTH) 
    {
        filled = BAR_WIDTH;
    }

    const char* color = (val > SEUIL) ? RED : GREEN;

    printf("  %s%-5s%s [%s", BOLD, label, RESET, color);
    for (int i = 0; i < BAR_WIDTH; i++)
    {
        putchar(i < filled ? '#' : '.');
    }
    printf("%s] ", RESET);

    /* Pourcentage coloré */
    printf("%s%5.1f%%%s\n", color, val, RESET);
}

/* ─────────────────────────────────────────────
   Boucle principale
   ───────────────────────────────────────────── */

int main(void) 
{
    printf("%s", BOLD);
    printf("  Surveillance des ressources système  (Ctrl+C pour quitter)\n");
    printf("%s", RESET);

    for (;;) 
    {
        double ram  = get_ram_percent();
        double cpu  = get_cpu_percent();
        double disk = get_disk_percent("/");

        /* Efface l'écran et réaffiche depuis le début */
        printf("%s", CLEAR);

        /* ── En-tête ── */
        printf("%s  ╔══════════════════════════════════════════╗%s\n", CYAN, RESET);
        printf("%s  ║   Surveillance des ressources système    ║%s\n", CYAN, RESET);
        printf("%s  ╚══════════════════════════════════════════╝%s\n\n", CYAN, RESET);

        /* ── Barres ── */
        print_bar("RAM",  ram);
        print_bar("CPU",  cpu);
        print_bar("DISK", disk);

        /* ── Alertes ── */
        int alerte = (ram > SEUIL || cpu > SEUIL || disk > SEUIL);
        if (alerte) 
        {
            printf("\n%s  ┌─ ALERTES ──────────────────────────────┐%s\n", YELLOW, RESET);
            if (ram  > SEUIL) printf("%s  │  ⚠ RAM  : %5.1f%% (> %d%%)               │%s\n", YELLOW, ram,  SEUIL, RESET);
            if (cpu  > SEUIL) printf("%s  │  ⚠ CPU  : %5.1f%% (> %d%%)               │%s\n", YELLOW, cpu,  SEUIL, RESET);
            if (disk > SEUIL) printf("%s  │  ⚠ DISK : %5.1f%% (> %d%%)               │%s\n", YELLOW, disk, SEUIL, RESET);
            printf("%s  └────────────────────────────────────────┘%s\n", YELLOW, RESET);
        }

        fflush(stdout);

        /* Pause jusqu'au prochain rafraîchissement
           (on soustrait les ~150 ms déjà consommés par get_cpu_percent) */
        usleep((REFRESH_MS - 150) * 1000);
    }

    return 0;
}
