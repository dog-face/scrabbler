//  /$$   /$$
// | $$  /$$/
// | $$ /$$/ 
// | $$$$$/  
// | $$  $$  
// | $$\  $$ 
// | $$ \  $$
// |__/  \__/
//KOPIMI

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define DICTLOAD 45428
#define DICTSIZE 38637
#define MAXLINE 30


int load_dictionary();
void print_dictionary();

int get_letters();
//int get_num_letters(); antiquated

int get_num_total_permutations();
int factorial(int n);
int nPr(int n, int r);

int malloc_letters();
int malloc_dictionary();
int malloc_permutations(int num_p, int num_l);
int malloc_possible_words(int num_p, int num_l);
int malloc_sorted_words();

void free_letters();
void free_dictionary();
void free_permutations(int num_p);
void free_possible_words(int num_p);
void free_sorted_words();

int generate_permutations(char* str, int len, int total);
void permute(char *str, int i, int n);
void swap (char *x, char *y);
void print_permutations(int num);

void generate_solutions();
void check_word(char* word);
static int binsearch(char *value);
int check_if_new(char* possible_solution);
void check_for_word(char* word);

void print_solutions();
int calculate_points(char* word);
int sort_solutions();
int compare_s_words(const void* a, const void* b);
int compare_words(const void* a, const void* b);

void dump_line(FILE * fp);

int run_main(char run);

//global vars
int num_letters;
int num_total_permutations;

char** dictionary;
char* letters;//[MAXLETTERS+1];
char **permutations;
int permute_iter = 0;
char **possible_words;

static int num_matches;

char scrabble = 'n';
int minletters = 3;
int maxletters = 7;

char** sorted_words;//[5][7];
int* sorted_points;
//end global vars

int main(){
	
	//AUTOMATIC SETUP:
	if(!malloc_dictionary()){
		printf("Error mallocing dictionary!!!\n");
		return 0;
	}
	if(!load_dictionary()){
		printf("Error loading dictionary!!!\n");
		return 0;
	}

	//GET USER INPUT SETUP:
	printf("Are you playing standard Scrabble? (y/n) ");
	scrabble = fgetc(stdin);
	dump_line(stdin);
	if(scrabble != 'y'){
		char m_letters[10];
		printf("Enter minimum characters: ");
		fgets(m_letters, 10, stdin);
		minletters = atoi(m_letters);
		if(!(minletters > 0)){
			printf("invalid minletters!!!\n");
			return 0;
		}
	
		char M_letters[10];
		printf("Enter maximum characters: ");
		fgets(M_letters, 10, stdin);
		maxletters = atoi(M_letters);
		if(!(maxletters > 0)){
			printf("invalid maxletters!!!\n");
			return 0;
		}
		if(!(maxletters >= minletters)){
			printf("error: !(maxletters >= minletters)\n");
			return 0;
		}
	}


	//MALLOCS BASED ON USER INPUT:
	num_total_permutations = get_num_total_permutations(maxletters);

	/*if(!malloc_permutations(num_total_permutations, maxletters)){
		printf("Error Mallocing permutation array!!! \n");
		return 0;
	}*/
	if(!malloc_possible_words(num_total_permutations, maxletters+1)){
		printf("Error Mallocing possible words array!!! \n");
		return 0;
	}

	if(!malloc_letters()){
		printf("Error mallocing letter!!!\n");
		return 0;
	}

	//RUN THE MAIN PROGRAM:
	run_main('y');

	//CLEANUP:
	printf("Exiting\n");

	free_letters();
	//free_permutations(num_total_permutations);
	free_possible_words(num_total_permutations);
	free_dictionary();
	return 1;
}

int run_main(char run){
	while(run == 'y'){
		dump_line(stdin);
		if(!get_letters()){
			printf("Error getting user input!!!\n");
			return 0;
		}

		num_letters = strlen(letters);
		letters[num_letters] = '\0';//make sure string is null-terminated
	

		//GENERATE PERMUTATIONS OF USER INPUT:
		num_matches = 0;
		if(!generate_permutations(letters, num_letters, num_total_permutations)){
			printf("Error generating permutations!!! \n");
			return 0;
		}
		
		//FIND SOLUTIONS IN PERMUTATIONS:
		
		//generate_solutions();
		print_solutions();

		dump_line(stdin);
		printf("run again? (y/n) ");
		run = fgetc(stdin);
		
	}
	return 1;
}

//################
//PRINT SOLUTIONS
//################
//prints solutions sorted by scrabble point value
void print_solutions(){
	sort_solutions();

	printf("\n%i MATCHING WORDS:\n", num_matches);
	if(scrabble == 'y')
		printf("POINTS | WORD\n");
	int i;
	for(i = 0; i < num_matches; i++){
		if(scrabble == 'y'){
			printf("%i | %s\n", calculate_points(sorted_words[i]), sorted_words[i]);
		}
		else{
			printf("%s\n", sorted_words[i]);
		}
	}
	if(scrabble == 'y')
		printf("POINTS | WORD\n");
	printf("END MATCHING WORDS\n\n");
	free_sorted_words();
}

//sorts sorted_words array by scrabble point value
int sort_solutions(){
	if(!malloc_sorted_words()){
		printf("error mallocing sorted words!!!\n");
		return 0;
	}

	//copy words from possible_words arry to sorted_words array
	int i;
	for(i = 0; i < num_matches; i++){
		strncpy(sorted_words[i], possible_words[i], maxletters);
	}

	if(scrabble == 'y'){//if standard scrabble points
		//quicksort by standard scrabble points
		qsort(sorted_words, num_matches, sizeof(char*), compare_s_words);
	}
	else{//if not standard scrabble
		//quicksort alphabetically
		qsort(sorted_words, num_matches, sizeof(char*), compare_words);
	}
	return 1;
}

//compares two words based on their scrabble point value. 
int compare_s_words(const void* a, const void* b){
    const char *pa = *(const char**)a;
    const char *pb = *(const char**)b;
    return calculate_points((char*)pa) - calculate_points((char*)pb);
}

int compare_words(const void* a, const void* b){
    const char *pa = *(const char**)a;
    const char *pb = *(const char**)b;
    return strlen(pa) - strlen(pb);
}

//calculates scrabble point value for a word
int calculate_points(char* word){
	int points = 0;
	int i;
	int j = strlen(word);
	for(i = 0; i < j; i++){
		if(word[i] == 'e' || word[i] == 'a' || word[i] == 'i' || word[i] == 'o' || 
		   word[i] == 'n' || word[i] == 'r' || word[i] == 't' || word[i] == 'l' || 
		   word[i] == 's' || word[i] == 'u'){
			points += 1;
		}
		else if(word[i] == 'd' || word[i] == 'g'){
			points += 2;
		}
		else if(word[i] == 'b' || word[i] == 'c' || word[i] == 'm' || word[i] == 'p'){
			points += 3;
		}
		else if(word[i] == 'f' || word[i] == 'h' || word[i] == 'v' || word[i] == 'w' ||
			    word[i] == 'y'){
			points += 4;
		}
		else if(word[i] == 'k'){
			points += 5;
		}
		else if(word[i] == 'j' || word[i] == 'x'){
			points += 8;
		}
		else if(word[i] == 'q' || word[i] == 'z'){
			points += 10;
		}
	}
	return points;
}

//##################
//GENERATE SOLUTIONS
//##################

void generate_solutions(){
	num_matches = 0;
	int i;
	for(i = 0; i < num_total_permutations; i++){//for each word in permutations[]
		check_word(permutations[i]);
		/*int j;
		for(j = minletters; j <= maxletters; j++){
			char* possible_solution = (char*) malloc((j+1) * sizeof(char));
			strncpy(possible_solution, permutations[i], j);
			possible_solution[j] = '\0';
			int match_index = binsearch(possible_solution);
			if(match_index != -1){//not 0. special case. 
				//if this matches a word in the dictionary
				if(check_if_new(possible_solution)){//check if it's a new word. 
					//if it is
					strncpy(possible_words[num_matches], possible_solution, maxletters+1);
    				num_matches++;
				}
			}
		}*/
	}
}

void check_word(char* word){
	
	int max;
	if(strlen(word) < maxletters)
		max = strlen(word);
	else
		max = maxletters;
	int j;
	for(j = minletters; j <= max; j++){
		char* possible_solution = (char*) malloc((j+1) * sizeof(char));
		strncpy(possible_solution, word, j);
		possible_solution[j] = '\0';
		int match_index = binsearch(possible_solution); 
		if(match_index != -1){//not 0. special case. 
			//if this matches a word in the dictionary
			if(check_if_new(possible_solution)){//check if it's a new word. 
				//if it is
				strncpy(possible_words[num_matches], possible_solution, maxletters+1);
    			num_matches++;
			}
		}
		free(possible_solution);
	}
}

static int binsearch(char *value) {
 int position;
 int begin = 0; 
 int end = DICTSIZE - 1;
 int cond = 0;
 int word_len;
 int max_len;
 word_len = strlen(value);

 while(begin <= end) {
  	position = (begin + end) / 2;
  	int dict_len = strlen(dictionary[position]);
  	if(dict_len > word_len){
  		max_len = dict_len;
  	}
  	else{
  		max_len = word_len;
  	}
  	
  	if((cond = strncmp(dictionary[position], value, max_len)) == 0){
  		//printf("found match: str: %s, dict: %s\n", value, dictionary[position]);
  		return position;
  	}
  	else if(cond < 0)
  	 begin = position + 1;
  	else
  	 end = position - 1;
 }
 
 return -1;//not 0 for failure, because 0 is the first word in dictionary
}

int check_if_new(char* possible_solution){
	int k;
	int new = 1;
	for(k = 0; k < num_matches; k++){//scan through all matches so far. 
		if(strncmp(possible_words[k], possible_solution, num_letters) == 0){
			//if this one is already in the list
			new = 0;//it's not new. 
			break;
		}
	}
	return new;
}

//for testing dictionary lookup. 
void check_for_word(char* word){
	int index = binsearch(word);
	if(index != -1){
		printf("dict[%i] = %s\n", index, dictionary[index]);
	}
	else{
		printf("no match!\n");
	}
}

//#####################
//GENERATE PERMUTATIONS
//#####################

//generate all possible permutations of length (minletters ... len)
int generate_permutations(char* str, int len, int total){
	num_matches = 0;
	//permute_iter = 0;
	permute(str, 0, len-1);
	return 1;
}

/* Function to print permutations of string
   This function takes three parameters:
   1. String
   2. Starting index of the string
   3. Ending index of the string. */
void permute(char *str, int i, int n) 
{	
   	int j; 
   	if (i == n){
    	//strncpy(permutations[permute_iter], str, maxletters);
    	check_word(str);
    	//permute_iter++;
 	}
   	else
   	{
        for (j = i; j <= n; j++)
    	{
          	swap((str+i), (str+j));
          	permute(str, i+1, n);
          	swap((str+i), (str+j)); //backtrack
       	}
   	}
}

/* Function to swap values at two pointers */
void swap (char *x, char *y){
    char temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

//for testing
void print_permutations(int num){
	int i;
	for(i = 0; i < num; i++){
		printf("permtuation %i: %s", i, permutations[i]);
	}
}

//################################
//CALCULATE NUMBER OF PERMUTATIONS
//################################

//number of total permutations
int get_num_total_permutations(int n_letters){
	int total = 0;
	return nPr(n_letters, n_letters);
}

//permutation function
int nPr(int n, int r){
	int permutations;
	permutations = factorial(n)/factorial(n-r);
	//n! / (n - r)! 
	return permutations;
}

//factorial function
int factorial(int n){
	if(n == 0){
		return 1;
	}
	else{
		return n * factorial(n-1);
	}
}

//##############
//GET USER INPUT
//##############
//takes user input letters
int get_letters(){
	printf("Input longer than %i letters will be truncated.\n", maxletters);
	printf("Enter your scrabble letters: ");
	fgets(letters, maxletters+1, stdin);
	printf("Your input after truncation: %s\n", letters);
	return 1;
}

//check number of input characters
/*
int get_num_letters(char* input_str){
	int i = 0;
	while(input_str[i] != '\n' && input_str[i] != '\0'){
		i++;
	}
	return i;
}*/
//ANTIQUATED BY STRLEN

void dump_line(FILE * fp) {
    int ch;
    while ((ch = fgetc(fp)) != EOF && ch != '\n') {
        /* null body */;
    }
}

//DICTIONARY METHODS
//loads the dictionary into an array
int load_dictionary(){
	FILE* fp = fopen("words.txt","r");
	rewind(fp);
	char tmpstring[MAXLINE];
	int i = 0;
	while(fgets(tmpstring, MAXLINE, fp) && i < DICTLOAD){
		if(tmpstring[0] >= 97){
			strncpy(dictionary[i], tmpstring, MAXLINE);
			int last_letter = strlen(dictionary[i]) -2;
			dictionary[i][last_letter] = '\0';
			//printf("%i: dictionary[%i] = %s\n", last_letter, i, dictionary[i]);
			i++;
		}
	}
	//printf("DICTSIZE = %i", i);
	if(i != DICTSIZE){
		return 0;//something went wrong
	}
	return 1;
}

//for testing
void print_dictionary(){
	int j;
	for(j = 0; j < DICTSIZE; j++){
		printf("Word %i: %s", j, dictionary[j]);
	}
}

//########################
//MALLOC AND FREE METHODS
//########################
int malloc_letters(){
	letters = (char*) malloc(maxletters * sizeof(char));
	if(letters == NULL){
		return 0;
	}
	return 1;
}

void free_letters(){
	if(letters != NULL){
		free(letters);
	}
}

int malloc_dictionary(){
	dictionary = (char**) malloc(DICTSIZE * sizeof(char*));
	if(dictionary == NULL){
		return 0;//failed
	}
	int i;
	for(i = 0; i < DICTSIZE; i++){
		dictionary[i] = (char*) malloc(MAXLINE * sizeof(char));
		if(dictionary[i] == NULL){
			return 0;//failed
		}
	}
	return 1;
}

void free_dictionary(){
	int i;
	for(i = 0; i < DICTSIZE; i++){
		if(dictionary[i] != NULL){
			free(dictionary[i]);
		}
	}
	if(dictionary != NULL){
		free(dictionary);
	}
}

int malloc_possible_words(int num_p, int num_l){
	//possible_words = (char**) malloc(num_p * sizeof(char*));
	possible_words = (char**) malloc(100 * sizeof(char*));
	if(possible_words == NULL){
		return 0;//failed
	}
	int i;
	for ( i = 0; i < 100; i++ )
	{
	    possible_words[i] = (char*) malloc(num_l * sizeof(char));
	    if(possible_words[i] == NULL){
	    	return 0;//failed
	    }
	}
	return 1;
}

void free_possible_words(int num_p){
	int i;
	for(i = 0; i < 100; i++){//free each string in the array
		if(possible_words[i] != NULL){
			free(possible_words[i]);
		}
	}
	if(possible_words != NULL){
		free(possible_words);//free the array itself
	}

}

//mallocs space for the permutations 2d array. 
int malloc_permutations(int num_p, int num_l){
	permutations = (char**) malloc(num_p * sizeof(char*));
	if(permutations == NULL){
		return 0;//failed
		printf("malloc_permutations: Error\n");
	}

	int i;
	for ( i = 0; i < num_p; i++ )
	{
	    permutations[i] = (char*) malloc(num_l * sizeof(char));
	    if(permutations[i] == NULL){
	    	return 0;//failed
	    }
	}
	return 1;
}

void free_permutations(int num_p){
	int i;
	for(i = 0; i < num_p; i++){//free all strings in array
		if(permutations[i] != NULL){
			free(permutations[i]);
		}
	}
	if(permutations != NULL){
		free(permutations);//free the array itself
	}
}

int malloc_sorted_words(){
	sorted_words = (char**) malloc(num_matches * sizeof(char*));
	if(sorted_words == NULL)
		return 0;
	int i;
	for(i = 0; i < num_matches; i++){
		sorted_words[i] = (char*) malloc(maxletters * sizeof(char));
		if(sorted_words[i] == NULL)
			return 0;
	}
	return 1;
}	
void free_sorted_words(){
	int i;
	for(i = 0; i < num_matches; i++){
		if(sorted_words[i] != NULL)
			free(sorted_words[i]);
	}
	if(sorted_words != NULL)
		free(sorted_words);
}
