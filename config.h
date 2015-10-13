#ifndef CONFIG_H_
#define CONFIG_H_

struct options {
	char *username;
	char *password;
};

void init_options(struct options *);
void free_options(struct options *);
void parse_config(struct options *, const char *);

#endif /* CONFIG_H_ */
