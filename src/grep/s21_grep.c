#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

typedef struct {
    int flag_i, flag_n, flag_s, flag_c, flag_v,
            flag_l, flag_h, flag_o, num_of_patterns, num_of_files;
    char **patterns, **files;
} flag_matr;

void read_flag_e(char *str,  flag_matr *matrix) {
    matrix -> patterns = (char**)realloc(matrix -> patterns, (matrix -> num_of_patterns + 1) * sizeof(char*));
    matrix -> patterns[matrix -> num_of_patterns] = (char*)malloc(strlen(str) +1);
    strcpy(matrix -> patterns[matrix -> num_of_patterns], str);
    matrix -> num_of_patterns++;
}

int read_flag_f(char *str, flag_matr *matrix) {
    FILE *file;
    int err = 0;
    if ((file = fopen(str, "r")) == NULL) {
        printf("s21_grep: %s: No such file or directory", str);
        err++;
    } else {
        char *tmp_str = malloc(10);
        size_t tmp_str_len = 10;
        while (1) {
            if (getline(&tmp_str, &tmp_str_len, file) == -1) break;
            if (tmp_str[strlen(tmp_str)-1] == '\n') tmp_str[strlen(tmp_str)-1] = '\0';
            read_flag_e(tmp_str, matrix);
        }
        free(tmp_str);
        fclose(file);
    }
    return err;
}

void read_file_name(char *str,  flag_matr *flags) {
    flags -> files = (char**)realloc(flags -> files, (flags -> num_of_files + 1) * sizeof(char*));
    flags -> files[flags -> num_of_files] = (char*)malloc(strlen(str) +1);
    strcpy(flags -> files[flags -> num_of_files], str);
    flags -> num_of_files++;
}

int parser(int argc, char **argv, flag_matr *flags) {
    int err_num = 0, flags_e_f = 0;
    for (int i = 1; i < argc && err_num == 0; i++) {
        if (strcmp(argv[i - 1], "-e") == 0 || strcmp(argv[i - 1], "-f") == 0) continue;
        if (argv[i][0] == '-' && strcmp(argv[i], "-") != 0) {
            for (int j = 1; argv[i][j] != '\0' && err_num == 0; j++) {
                switch (argv[i][j]) {
                    case 'i': flags->flag_i = 1; break;
                    case 'e': {
                        read_flag_e(argv[i + 1], flags);
                        flags_e_f++;
                        break;
                    }
                    case 'n': flags->flag_n = 1; break;
                    case 's': flags->flag_s = 1; break;
                    case 'c': flags->flag_c = 1; break;
                    case 'v': flags->flag_v = 1; break;
                    case 'l': flags->flag_l = 1; break;
                    case 'h': flags->flag_h = 1; break;
                    case 'f': {
                        err_num = read_flag_f(argv[i + 1], flags);
                        flags_e_f++;
                        break;
                    }
                    case 'o': flags->flag_o = 1; break;
                    default: printf("%s: invalid option -- %c", argv[0], argv[i][j]); return 2;
                }
            }
        } else {
            if (!flags_e_f) {
                read_flag_e(argv[i], flags);
                flags_e_f++;
            } else {
                read_file_name(argv[i], flags);
            }
        }
    }
    if (flags->flag_c || flags->flag_l) flags->flag_o = 0;
    return err_num;
}

void flag_c_work(flag_matr flags, char *file_mame, int c_cou) {
    if (flags.flag_c) {
        if (!flags.flag_h && flags.num_of_files > 1) printf("%s:", file_mame);
        if (flags.flag_l) {
            if (c_cou > 0) {
                printf("1\n");
            } else {
                printf("0\n");
            }
        } else {
            printf("%d\n", c_cou);
        }
    }
}

void flag_l_work(flag_matr flags, char *file_mame) {
    if (flags.flag_l) {
        printf("%s\n", file_mame);
    }
}

int do_comp(flag_matr flags, regex_t *regex, int id) {
    int val;
    if (flags.flag_i)
        val = regcomp(regex, flags.patterns[id], REG_ICASE);
    else
        val = regcomp(regex, flags.patterns[id], 0);
    return val;
}

void print_with_no_fl(flag_matr flags, char *file_mame, char *str, int cou_of_str) {
    if (!flags.flag_l && !flags.flag_c && !flags.flag_o) {
        if (!strcmp(file_mame, "-")) {
            if (!flags.flag_h && flags.num_of_files > 1) printf("(standard input):");
        } else {
            if (!flags.flag_h && flags.num_of_files > 1) printf("%s:", file_mame);
        }
        if (flags.flag_n) printf("%d:", cou_of_str);
        if (str[strlen(str) - 1] == '\n') str[strlen(str) - 1] = '\0';
        printf("%s\n", str);
    }
}

void flag_o_work(flag_matr *flags, char *file_mame, int n_cou, char *str, int fl) {
    if (!flags->flag_h && flags->num_of_files > 1 && !fl) printf("%s:", file_mame);
    if (flags->flag_n && !fl) printf("%d:", n_cou);
    printf("%s\n", str);
}

void text_out(char **argv, flag_matr *flags) {
    regex_t regex;
    for (int i = 0; i < flags->num_of_files; i++) {
        FILE *file;
        if ((file = !strcmp(flags->files[i], "-") ? stdin : fopen(flags->files[i], "r")) == NULL) {
            if (!flags->flag_s) {
                printf("%s: %s: No such file or directory\n", argv[0], flags->files[i]);
            }
        } else {
            char *tmp_str = malloc(10);
            int flag_n_count = 0, flag_c_count = 0;
            size_t tmp_str_len = 10;
            while (getline(&tmp_str, &tmp_str_len, file) != -1) {
                int flag_o_w = 0;
                flag_n_count++;
                for (int j = 0; j < flags->num_of_patterns; j++) {
                    if (!do_comp(*flags, &regex, j) || flags->patterns[j][0] == '\0') {
                        int val = regexec(&regex, tmp_str, 0, NULL, 0);
                        regfree(&regex);
                        if (flags->patterns[j][0] == '\0') val = 0;
                        if (!val) {
                            if (!flags->flag_v) {
                                flag_c_count++;
                                if (flags->flag_o) {
                                    flag_o_work(flags, flags->files[i], flag_n_count,
                                                flags->patterns[j], flag_o_w);
                                    flag_o_w++;
                                } else {
                                    print_with_no_fl(*flags, flags->files[i],
                                                     tmp_str, flag_n_count);
                                }
                            }
                            if (!flags->flag_o)
                                break;
                        } else {
                            if (flags->flag_v) {
                                if (j == flags->num_of_patterns - 1) {
                                    flag_c_count++;
                                    if (flags->flag_o) {
                                        flag_o_work(flags, flags->files[i], flag_n_count,
                                                    flags->patterns[j], flag_o_w);
                                        flag_o_w++;
                                    } else {
                                        print_with_no_fl(*flags, flags->files[i],
                                                         tmp_str, flag_n_count);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            flag_c_work(*flags, flags->files[i], flag_c_count);
            flag_l_work(*flags, flags->files[i]);
            free(tmp_str);
        }
    }
}

void init_struct(flag_matr *flags_and_matrix) {
    flags_and_matrix -> flag_s = 0;
    flags_and_matrix -> flag_i = 0;
    flags_and_matrix -> flag_n = 0;
    flags_and_matrix -> flag_c = 0;
    flags_and_matrix -> flag_v = 0;
    flags_and_matrix -> flag_l = 0;
    flags_and_matrix -> flag_h = 0;
    flags_and_matrix -> flag_o = 0;
    flags_and_matrix -> num_of_patterns = 0;
    flags_and_matrix -> patterns = NULL;
    flags_and_matrix -> files = NULL;
    flags_and_matrix -> num_of_files = 0;
}

int main(int argc, char ** argv) {
    flag_matr flags_and_matrix;
    init_struct(&flags_and_matrix);
    int err_num = parser(argc, argv, &flags_and_matrix);
    if (err_num) {
        if (err_num == 2) {
            printf("\nusage: s21_grep [-abcDEFGHhIiJLlmnOoqRSsUVvwxZ] [-A num] [-B num] [-C[num]]\n"
                   "        [-e pattern] [-f file] [--binary-files=value] [--color=when]\n"
                   "        [--context[=num]] [--directories=action] [--label] [--line-buffered]\n"
                   "        [--null] [pattern] [file ...]");
        }
    } else {
        text_out(argv, &flags_and_matrix);
    }
    for (int i = 0; i < flags_and_matrix.num_of_patterns; i++) {
        free(flags_and_matrix.patterns[i]);
    }
    for (int i = 0; i < flags_and_matrix.num_of_files; i++) {
        free(flags_and_matrix.files[i]);
    }
    free(flags_and_matrix.files);
    free(flags_and_matrix.patterns);
    return 0;
}
