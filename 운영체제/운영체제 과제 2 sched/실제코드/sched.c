/*
 * OS Assignment #2
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>

#include "sched.h"

/**
 * letter_list에 letter_node를 추가하는 함수입니다.
 * int line_index: 노드가 추가될 print_list를 가진 parsed_str_array의 배열 인덱스입니다.
 * char input_letter: 새로 추가될 노드의 문자입니다.
 */
void list_add(int line_index, char input_letter)
{
	letter_node* new_node = calloc(1,sizeof(letter_node));
	new_node->letter = input_letter;

	//만약 리스트의 길이가 0이라면 이는 새로 추가된 노드가 노드의 tail이자 head라는 의미입니다. 새로운 노드를 head로도 지정합니다.
	if(parsed_str_array[line_index].print_list.list_many == 0)
	{
		parsed_str_array[line_index].print_list.tail = new_node;
		parsed_str_array[line_index].print_list.head = new_node;
	}
	//새로운 노드를 리스트 끝에 연결하고 새로운 tail로 지정합니다.
	else
	{
		parsed_str_array[line_index].print_list.tail->next_node = new_node;
		parsed_str_array[line_index].print_list.tail = new_node;
	}

	parsed_str_array[line_index].print_list.list_many++;
}

/**
 * 리스트의 head에서부터 하나씩 노드의 문자를 가져오고 노드를 제거합니다.
 * int line_index: 노드가 추가될 print_list를 가진 parsed_str_array의 배열 인덱스입니다.
 * return return_letter: 노드에서 반환된 문자입니다. 만약 노드가 없다면 반환값은 null입니다.
 */
char list_pop(int line_index)
{
	//list_many가 0이면 가져올 노드가 없다는 뜻입니다. 0(NULL)을 반환합니다.
	if (parsed_str_array[line_index].print_list.list_many == 0)
	{
		return 0;
	}

	//반환할 문자 값을 가져옵니다.
	char return_letter = parsed_str_array[line_index].print_list.head->letter;

	//이제 head를 제거하고 다음 노드를 새로운 head로 지정합니다.
	letter_node *delete_node = parsed_str_array[line_index].print_list.head;
	parsed_str_array[line_index].print_list.head = parsed_str_array[line_index].print_list.head->next_node;
	parsed_str_array[line_index].print_list.list_many--;
	free(delete_node);

	return return_letter;
}

/**
 * 특정 문자열에서 특정 문자의 갯수를 세는 코드입니다.
 * char* string: 특정 문자를 포함한 문자열
 * char letter: 찾는 문자
 */
int letter_cnt(char * string, char letter)
{
	int result = 0;
	 
	while (*string) //string안에 값이 NULL이면 문자열이 끝난것이므로 루프가 끝납니다.
	{
		if (letter == * string)
		{
			result++;
		}
		string++;
	}
	 
	return result;
}

/**
 * \n의 갯수 = 행의 갯수를 읽는 함수입니다. 이후에 이 행의 갯수 만큼 parseed_string 구조체의 배열을 동적으로 할당할 것입니다.
 */ 
void read_new_line_letter()
{
	line_many = 0;
	char * message = NULL;
	size_t size = 0;
	
	while (!feof(argv_file))
	{
		getline(&message,&size,argv_file);
		line_many++;
	}
	
	free(message);
	
	//이제 파일의 지시자를 맨 앞으로 옮깁시다.
	fseek(argv_file,0,SEEK_SET);
}

/**
* data 파일을 읽고, 프로세스의 갯수, 파싱과정을 거치면서 분석에 필요한 데이터를 모읍니다.
*/
void read_data_file()
{
	correct_process_many = 0; //정확한 프로세스 갯수를 세는데 쓰이는 전역변수를 초기화 합니다.
	last_process_index = -1; //last_process_index는 시작할 때 -1을 삽입합니다. last_process_index가 -1 이라는 것은 아직 정상적인 프로세스 정보를 파싱한 적이 없다는 뜻입니다.
	size_t size = 0; //getline함수를 쓰기위해서 존재하는 변수입니다. 이 안에 값이 쓰일 일은 없습니다.
	read_new_line_letter();
	
	input_string_array = calloc(line_many,sizeof(char*));
	parsed_str_array = calloc(line_many,sizeof(parsed_string));

	int line_index; //각 줄을 탐색하는데 쓰일 인덱스입니다.
	for (line_index = 0; !feof(argv_file); line_index++)
	{
		getline(&input_string_array[line_index],&size,argv_file);
		if (input_string_array[line_index][0] != '#' && input_string_array[line_index][0] != '\n' && strlen(input_string_array[line_index]) > 0)
		{
			parse_string(line_index);
			
		}
		else
		{
			parsed_str_array[line_index].program_id[0] = '#';
		}
	}

	//이제 correct_process_many에 맞춰 Ready Que의 길이를 지정합니다.
	ready_que = calloc(correct_process_many,sizeof(int));
	ready_que_many = 0;
}

 /**
  * 문자열의 앞뒤 공백을 제거하는 함수입니다.
  * 어디까지나 앞과 뒤만 제거하므로, 사이에 있는 공백은 제거하지 않습니다.
  * char* string: 공백을 제거할 문자열입니다.
  */
void remove_string_space(char* string)
{
	int string_length = strlen(string);
	int string_index = 0;
	int last_letter_index = 0; //탐색 중 마지막으로 문자가 있던 인덱스입니다.
	int next_letter_index = 0; //last_letter_index가 변경된 이후, 처음으로 문자가 있던 인덱스입니다. 만일 이 인덱스가 strlen과 같다면, 이는 문자가 더 없다는 뜻입니다.

	for (string_index = 0; string_index < string_length; string_index++)
	{
		if (!isspace((int)string[string_index])) //공백이 아니면 last_letter_index값을 변경합니다.
		{
			last_letter_index = string_index;
		}
		else if (last_letter_index == 0) //공백이지만 아직까지 문자가 발견된 적은 없습니다. 즉, 문자열 앞부분에 공백이 있습니다. 이를 제거합니다.
		{
			int remove_index; //제거할 때 쓰일 인덱스입니다.
			for (remove_index = string_index + 1; remove_index <= string_length; remove_index++) //현재 위치부터 이 뒤에 있는 값을 앞으로 넣어야합니다. 이를 위한 함수입니다.
			{
				string[remove_index - 1] = string[remove_index];
			}
			string_length--;
			string_index--; //문자열을 한칸 앞으로 당겼으므로, 한번더 같은 곳을 검사해야합니다. 이를 위해 string_index를 1 감소시킵니다.
		}
		else //이번에는 공백이 문자열 사이에 있는지 끝에 있는지를 검사해야합니다.
		{
			if (next_letter_index < string_index) //next_letter_index가 string_index보다 작다면 이는 공백 다음에 문자가 있는지 탐색을 하지않았다는 뜻입니다. 탐색을 시작합니다.
			{
				for (next_letter_index = string_index; next_letter_index < string_length; next_letter_index++)
				{
					if (!isspace((int)string[next_letter_index])) //문자가 발견되었습니다. 바로 
						break;
				}

				if (isspace((int)string[string_length-1]))
					next_letter_index++;
			}

			if (next_letter_index >= string_length) //next_letter_index가 string_length보다 크거나 같다면 공백이 문자열 뒤에 존재한다는 뜻입니다. 바로 제거합니다.
			{
				string[last_letter_index+1] = string[string_length];
				string_length = last_letter_index - 1;
			}
		}
	}
}

/**
 * id가 형식에 맞는지 체크합니다.
 * return: int 오류 코드
 * 0: 문제 없음
 * 1: 오류 발견 에러를 출력해야합니다.
 * 2: 중복된 아이디입니다.
 */
int check_id(char* seperated_string)
{
	int length = strlen(seperated_string); //검사할 문자열의 길이입니다.
	int check_index; //중복을 검색할 때 쓰는 인덱스입니다.
	if (length != 2)
	{
		return 1;
	}
	//일단 두 글자가 각각 영문자(소문자 포함)와 숫자인지 검사합니다.
	if (isalnum(seperated_string[0]) && isalnum(seperated_string[1]))
	{
		//이제 두 문자중에 소문자가 있는지 검사합니다. 이렇게 해서 id 형식 검사가 끝납니다.
		if (97 <= seperated_string[0] && seperated_string[0] <= 122)
		{
			return 1;
		}
		if (97 <= seperated_string[1] && seperated_string[1] <= 122)
		{
			return 1;
		}
	
		//형식이 맞다는 것을 확인했으니 이제 중복인지 아닌지 검사합니다.
		for (check_index = 0; check_index < line_many; check_index++)
		{
			if (strcmp(seperated_string, parsed_str_array[check_index].program_id) == 0)
			{
				return 2;
			}
		}
		
		return 0;
	}
	
	//길이가 2가 아니라면 이는 잘못된 형식의 id입니다.
	return 1;
}

/**
 * arrive_time이 형식에 맞는지 체크합니다. 이 때, last_process_index가 -1이 아니면 이 값을 인덱스로 parsed_str_array에 접근해서 arrive_time을 비교합니다.
 * return: int 오류 코드
 * 0: 문제 없음
 * 1: 오류 발견 에러를 출력해야합니다.
 */
int check_arrive_time(char* seperated_string)
{
	int arrive_time = atoi(seperated_string);
	if (0 <= arrive_time && arrive_time <= 30)
	{
		if (last_process_index >= 0)
		{
			if (arrive_time < parsed_str_array[last_process_index].arrive_time)
			{
				return 1;
			}
		}
		return 0; //문제가 없습니다. 다음으로 넘어갑니다.
	}	
	return 1;
}


/**
 * service time이 형식에 맞는지 체크합니다.
 * return: int 오류 코드
 * 0: 문제 없음
 * 1: 오류 발견 에러를 출력해야합니다.
 */
int check_service_time(char* seperated_string)
{
	int service_time = atoi(seperated_string);
	if (1 <= service_time && service_time <= 30)
	{
		return 0;
	}
	return 1;
}


/**
 * priority가 형식에 맞는지 체크합니다.
 * return: int 오류 코드
 * 0: 문제 없음
 * 1: 오류 발견 에러를 출력해야합니다.
 */
int check_priority(char* seperated_string)
{
	int priority = atoi(seperated_string);
	if (1 <= priority && priority <= 10)
	{
		return 0;
	}
	return 1;
}

/**
* 입력된 한 줄을 파싱하는 함수입니다. 오류가 있으면 1을 반환합니다.
*/
int parse_string(int line_index)
{
	char * copied_string = NULL; //strsep함수를 사용하기위해서는 내용이 변해도 문제없는 문자열이 필요합니다. 이를 위해 strdup함수를 이용해서 문자열을 복제할 것입니다.
	char * seperated_string = NULL; //strsep함수로 분리된 문자열은 여기에 저장됩니다. 이후에 값을 parse_str_array의 원소에 저장할 것입니다.
	char * delimiter = " "; //구분자 변수입니다.
	
	int check_result = 0; //파싱후의 구문을 검사한 결과 값입니다.
	
	copied_string = strdup(input_string_array[line_index]);
	remove_string_space(copied_string);

	//형식에 맞는지 검사합니다. 형식은 ' '가 3개 있는 형태여야합니다.
	if (letter_cnt(input_string_array[line_index],*delimiter) != 3)
	{
		fprintf(stderr,"invalid format in line %d, ignored\n",line_index + 1);
	}
	
	seperated_string = strsep(&copied_string, delimiter);
	remove_string_space(seperated_string);
	if ((check_result = check_id(seperated_string))) //먼저 id의 형식이 맞는지 확인합니다. 문제가 있으면 if는 참입니다.
	{
		if (check_result == 1)
			fprintf(stderr,"invalid process id ‘%s’ in line %d, ignored\n",seperated_string, line_index + 1);
		else if (check_result == 2)
			fprintf(stderr,"duplicate process id ‘%s’ in line %d, ignored\n",seperated_string,line_index + 1);
		parsed_str_array[line_index].program_id[0] = '#'; //오류가 있는 칸임을 알리는 값으로 id의 맨 앞을 #으로 지정합니다.
		return 1;
	}
	
	//문제가 없으면 id를 구조체에 저장합니다.
	strcpy(parsed_str_array[line_index].program_id,seperated_string);
	
	seperated_string = strsep(&copied_string,delimiter);
	remove_string_space(seperated_string);
	
	if (check_arrive_time(seperated_string))
	{
		fprintf(stderr,"invalid arrive-time ‘%s’ in line %d, ignored\n",seperated_string, line_index + 1);
		parsed_str_array[line_index].program_id[0] = '#'; //오류가 있는 칸임을 알리는 값으로 id의 맨 앞을 #으로 지정합니다.
		return 1;
	}
	
	parsed_str_array[line_index].arrive_time = atoi(seperated_string);
	
	seperated_string = strsep(&copied_string,delimiter);
	remove_string_space(seperated_string);
	if (check_service_time(seperated_string))
	{
		fprintf(stderr,"invalid service-time ‘%s’ in line %d, ignored\n",seperated_string, line_index + 1);
		parsed_str_array[line_index].program_id[0] = '#'; //오류가 있는 칸임을 알리는 값으로 id의 맨 앞을 #으로 지정합니다.
		return 1;
	}
	
	parsed_str_array[line_index].service_time = atoi(seperated_string);
	parsed_str_array[line_index].remain_time = parsed_str_array[line_index].service_time;
	
	seperated_string = strsep(&copied_string,delimiter);
	remove_string_space(seperated_string);
	if (check_priority(seperated_string))
	{
		fprintf(stderr,"invalid priority ‘%s’ in line %d, ignored\n",seperated_string, line_index + 1);
		parsed_str_array[line_index].program_id[0] = '#'; //오류가 있는 칸임을 알리는 값으로 id의 맨 앞을 #으로 지정합니다.
		return 1;
	}
	
	parsed_str_array[line_index].priority = atoi(seperated_string);
	
	
	correct_process_many++;
	last_process_index = line_index;
	
	return 0;
}

/**
 * SJF알고리즘에 따라 다음에 실행할 프로세스의 이름을 가져오는 함수입니다.
 * return fast_index 알고리즘이 선택한 프로세스의 ready_que 인덱스입니다.
 * -1:도착한 프로세스가 없습니다. 일단은 올 때까지 실행을 하지 않습니다.
 * 0이상:해당 인덱스의 프로세스가 선택되었습니다. 이를 실행시킵니다.
 */
int select_process_SJF()
{
	int que_index; //que를 탐색할 때 쓸 인덱스입니다.
	int fast_index; //현재 실행중인 프로세스 + Ready Que 중에서 가장 Shortes job인 프로세스의 parsed_string 인덱스입니다.
	int fast_service_time; //현재 실행중인 프로세스 + Ready Que 중에서 가장 Shortes job인 프로세스의 service_time입니다.
	int current_service_time; //현재 실행중인 프로세스의 service_time입니다. 실행 중인 프로세스가 없다면 최대값 + 1로 초기화합니다.
	
	if(cur_run_proc < 0) //현재 실행중인 프로세스가 없습니다.
	{
		current_service_time = 31; //현재 Ready Que에 있는 프로그램 중에서 조건에 맞는 프로그램을 찾으면 됩니다.
	}
	else
	{
		current_service_time = parsed_str_array[ready_que[cur_run_proc]].service_time;
	}

	fast_index = cur_run_proc; //먼저 기본 값으로는 현재 실행 중인 프로세스가 가장 짧다고 가정합니다.
	fast_service_time = current_service_time;
	for (que_index = 0; que_index < ready_que_many; que_index++) //이후에 모든 프로세스를 검사하면서 더 짧은 프로세스가 있는지 찾습니다.
	{
		if (parsed_str_array[ready_que[que_index]].service_time < fast_service_time)
		{
			fast_service_time = parsed_str_array[ready_que[que_index]].service_time;
			fast_index = que_index;
		}
	}

	return fast_index;
}

/**
 * SRT알고리즘에 따라 다음에 실행할 프로세스의 이름을 가져오는 함수입니다.
 * return fast_index 알고리즘이 선택한 프로세스의 ready_que 인덱스입니다.
 * -1:도착한 프로세스가 없습니다. 일단은 올 때까지 실행을 하지 않습니다.
 * 0이상:해당 인덱스의 프로세스가 선택되었습니다. 이를 실행시킵니다.
 */
int select_process_SRT()
{
	int que_index; //que를 탐색할 때 쓸 인덱스입니다.
	int fast_index; //현재 실행중인 프로세스 + Ready Que 중에서 가장 Shortes job인 프로세스의 parsed_string 인덱스입니다.
	int fast_remain_time; ////현재 실행중인 프로세스 + Ready Que 중에서 가장 Shortest remain time인 프로세스의 remain_time입니다.
	int current_remain_time; //현재 실행중인 프로세스의 remain_time입니다. 실행 중인 프로세스가 없다면 최대값 + 1로 초기화합니다.
	
	if(cur_run_proc < 0) //현재 실행중인 프로세스가 없습니다.
	{
		current_remain_time = 31; //현재 Ready Que에 있는 프로그램 중에서 조건에 맞는 프로그램을 찾으면 됩니다.
	}
	else
	{
		current_remain_time = parsed_str_array[ready_que[cur_run_proc]].remain_time;
	}

	fast_index = cur_run_proc; //먼저 기본 값으로는 현재 실행 중인 프로세스가 가장 짧다고 가정합니다.
	fast_remain_time = current_remain_time;
	for (que_index = 0; que_index < ready_que_many; que_index++) //이후에 모든 프로세스를 검사하면서 더 짧은 프로세스가 있는지 찾습니다.
	{
		if (parsed_str_array[ready_que[que_index]].remain_time < fast_remain_time)
		{
			fast_remain_time = parsed_str_array[ready_que[que_index]].remain_time;
			fast_index = que_index;
		}
	}

	return fast_index;
}

/**
 * RR알고리즘에 따라 다음에 실행할 프로세스의 이름을 가져오는 함수입니다.
 * 저의 알고리즘은 동시기에 타임 아웃 처리가 프로그램 도착 처리보다 나중에 이루어지므로, 
 * 타임 아웃과 도착이 서로 다른 프로세스에서 동시에 발생했을 때, 도착한 프로세스가 먼저 작동합니다. 
 * 따라서, 예제와는 다른 결과가 나옵니다.
 * return fast_index 알고리즘이 선택한 프로세스의 ready_que 인덱스입니다.
 * -1:도착한 프로세스가 없습니다. 일단은 올 때까지 실행을 하지 않습니다.
 * 0이상:해당 인덱스의 프로세스가 선택되었습니다. 이를 실행시킵니다.
 */
int select_process_RR()
{
	int que_index; //que를 탐색할 때 쓸 인덱스입니다.
	int find_index;
	
	if (cur_run_proc < 0) //현재 실행중인 프로세스가 없습니다.
	{
		if (ready_que_many < 0) //Ready Que에 Ready중인 프로세스가 없습니다.
		{
			return -1;
		}
		else
		{
			parsed_str_array[ready_que[0]].time_quantum = 1;
			return 0; //먼저 온 순서대로 시작해야합니다. 이에 따라서 타임퀀텀을 주고 1을 합니다.
		}
	}
	else
	{
		if (ready_que_many <= 1) //Ready Que에 Reday중인 프로세스가 없습니다.
		{
			return cur_run_proc; //기존 프로세스를 계속 이용합니다. 타임아웃은 유지됩니다.
		}
		else
		{
			parsed_str_array[ready_que[cur_run_proc]].time_quantum--; //먼저 타임 퀀텀을 -1 합니다.
			if (parsed_str_array[ready_que[cur_run_proc]].time_quantum == 0) //타임 아웃이 일어나는지 검사합니다.
			{
				//타임 아웃이 일어났습니다. 일단 기존에 실행하던 프로세스는 Ready Que의 끝자리에 들어가야하므로 이를 위한 백업을 해놓습니다.
				int backup_line_index = ready_que[cur_run_proc];
				//이제 앞으로 배열을 한칸씩 당겨서 초기화합니다. 이렇게 해서 바로 다음에 Ready Que에 들어온 프로세스가 실행할 차례가 됩니다.
				for (que_index = 0; que_index < ready_que_many; que_index++)
				{
					ready_que[que_index] = ready_que[que_index + 1];
				}
				//이제 백업해놓은 값을 Ready Que 끝에 저장합니다.
				ready_que[ready_que_many - 1] = backup_line_index;
				
				//마지막으로 새롭게 시작할 프로세스에게 타임 퀀텀을 1 부여합니다.
				parsed_str_array[ready_que[0]].time_quantum = 1;
				
				cur_run_proc = 0;
			}
			return cur_run_proc;
		}
	}
	
	return find_index;
}

/**
 * PR알고리즘에 따라 다음에 실행할 프로세스의 이름을 가져오는 함수입니다.
 * return fast_index 알고리즘이 선택한 프로세스의 ready_que 인덱스입니다.
 * -1:도착한 프로세스가 없습니다. 일단은 올 때까지 실행을 하지 않습니다.
 * 0이상:해당 인덱스의 프로세스가 선택되었습니다. 이를 실행시킵니다.
 */
int select_process_PR()
{
	int que_index; //que를 탐색할 때 쓸 인덱스입니다.
	int fast_index; //현재 실행중인 프로세스 + Ready Que 중에서 가장 Shortes job인 프로세스의 parsed_string 인덱스입니다.
	int fast_priority; ////현재 실행중인 프로세스 + Ready Que 중에서 가장 priority가 높은 프로세스의 priority입니다. 작은 값이 높은 우선순위입니다.
	int current_priority; //현재 실행중인 프로세스의 priority입니다. 실행 중인 프로세스가 없다면 최대값 + 1로 초기화합니다.
	
	if(cur_run_proc < 0) //현재 실행중인 프로세스가 없습니다.
	{
		current_priority = 6; //현재 Ready Que에 있는 프로그램 중에서 조건에 맞는 프로그램을 찾으면 됩니다.
	}
	else
	{
		current_priority = parsed_str_array[ready_que[cur_run_proc]].priority;
	}

	fast_index = cur_run_proc; //먼저 기본 값으로는 현재 실행 중인 프로세스의 우선순위가 가장 높다고 가정합니다.
	fast_priority = current_priority;
	for (que_index = 0; que_index < ready_que_many; que_index++) //이후에 모든 프로세스를 검사하면서 우선순위가 더 높은 프로세스가 있는지 찾습니다.
	{
		if (parsed_str_array[ready_que[que_index]].priority < fast_priority)
		{
			fast_priority = parsed_str_array[ready_que[que_index]].priority;
			fast_index = que_index;
		}
	}

	return fast_index;
}

/**
 * 현재 cpu시간에 시작하는 프로그램을 찾아서 그 인덱스를 ready_que에 저장합니다.
 */
void find_start_process()
{
	int line_index; //parsed_str_array를 탐색할 때 사용할 인덱스입니다.
	for (line_index = 0; line_index < line_many; line_index++)
	{
		if (parsed_str_array[line_index].program_id[0] == '#')
			continue;
		else if (current_cpu_time == parsed_str_array[line_index].arrive_time) //이제 도착 시간과 현재시간이 같은 프로그램이 존재하는지 비교합니다.
		{
			//Ready Que에 그 프로그램의 정보가 담긴 구조체의 인덱스=line_index를 저장합니다.
			ready_que[ready_que_many] = line_index;
			ready_que_many++;
		}
	}
}

/**
 * 현재 실행중인 프로세스가 종료될 순간인지 검사합니다. 종료될 순간이면 종료 시간을 저장하고, 현재 실행중인 프로세스가 없다고 알립니다.
 */
void check_end_process()
{
	int delete_index; //Ready Que에서 종료된 프로세스의 인덱스를 제거할 때 쓸 인덱스입니다.
	if (cur_run_proc < 0)
	{
		return;
	}
	parsed_str_array[ready_que[cur_run_proc]].remain_time--;
	if (parsed_str_array[ready_que[cur_run_proc]].remain_time <= 0)
	{
		parsed_str_array[ready_que[cur_run_proc]].complete_time = current_cpu_time; //먼저 종료시간을 등록합니다.
		for (delete_index = cur_run_proc + 1; delete_index < ready_que_many; delete_index++) //이제 종료된 프로세스를 큐에서 제거합니다. 이를 위해 뒤에서부터 하나씩 덮어씁니다.
		{
			ready_que[delete_index - 1] = ready_que[delete_index];
		}
		ready_que_many--; //마지막으로 ready_que_many를 축소합니다.
		cur_run_proc = -1;
		end_process_many++;
	}
}

/**
 * 각 프로세스들을 다른 알고리즘으로 검사하기위해서 parsed_string 구조체 변수의 remain_time 멤버 변수를 service_time과 동일하게 초기화합니다.
 */
void reset_remain_time()
{
	int line_index; //parsed_str_array를 탐색할 때 사용할 인덱스입니다.
	for (line_index = 0; line_index < line_many; line_index++)
	{
		if (parsed_str_array[line_index].program_id[0] == '#')
			continue;
		else
		{
			parsed_str_array[line_index].remain_time = parsed_str_array[line_index].service_time;
		}
	}
}

/**
 * 스케쥴링  결과를 출력하는 함수입니다. 여기에서 평균 완료시간과 평균 대기시간을 계산합니다.
 */
void print_result()
{
	int line_index; //parsed_str_array를 탐색할 때 사용할 인덱스입니다.
	int total_turnarround_time = 0; //각 프로세스의 종료 시간을 모두 합한 값입니다. 이 값을 correct_process_many로 나눈 값이 평균 완료 시간입니다.
	int total_service_time = 0; //각 프로세스의 실행시간을 모두 합한 값입니다. 이 값을 total_turnarround_time에서 빼고 correct_process_many로 나누면 평균 대기 시간이 나옵니다. 
	for (line_index = 0; line_index < line_many; line_index++)
	{
		if (parsed_str_array[line_index].program_id[0] == '#')
			continue;
	
		else
		{
			total_turnarround_time += parsed_str_array[line_index].complete_time - parsed_str_array[line_index].arrive_time;
			total_service_time += parsed_str_array[line_index].service_time;
			printf("%s  ",parsed_str_array[line_index].program_id);
			while(parsed_str_array[line_index].print_list.list_many > 0)
			{
				printf("%c",list_pop(line_index));
			}
			printf("\n");
		}
	}
	printf("CPU TIME: %d\n",current_cpu_time);
	printf("AVERAGE TURNARROUND TIME: %.2f\n", (float)total_turnarround_time / correct_process_many);
	printf("AVERAGE WAITING TIME: %.2f\n", ((float)(total_turnarround_time - total_service_time)) / correct_process_many);
}

/**
 * 이제 프로세스들을 실행합니다. 프로세스들은 한번의 실행(1밀리초)을 하고 그 후에 검사를 합니다. 검사하는 순서는
 * 1. 실행이 끝난 프로세스가 있는지 확인
 * 2. 시작할 프로세스가 있는지 확인(parsed_str_array 탐색)
 * 3. 실행중인 프로세스 중에서 해당 스케쥴링 알고리즘에 따라 선택된 프로세스를 실행
 *
 * int ALGORITHM:프로세스 스케쥴러의 스케쥴링 알고리즘입니다. 미리 지정된 한정 변수로 지정합니다.
 */
void process_run(int ALGORITHM)
{
	current_cpu_time = 0;
	cur_run_proc = -1;
	end_process_many = 0;
	int line_index; //parsed_str_array를 탐색할 때 사용할 인덱스입니다.
	
	//프로세스들을 검사하기에 앞서 먼저 parsed_str_array에서 remain_time을 리셋시킵니다.
	reset_remain_time();
	
	//correct_process_many 개의 프로세스를 검색할 때까지 반복문으로 검사합니다.
	while (1)
	{
		check_end_process();
		if (end_process_many >= correct_process_many)
		{
			break;
		}
		find_start_process();
		
		switch (ALGORITHM) //이제 주어진 알고리즘에 따라서 스케줄링을 합니다.
		{
			case 1:
			{
				if (cur_run_proc == -1) //SJF는 선점이 없으므로 실행 중인 프로세스가 없을 때만 스케줄러를 호출합니다.
					cur_run_proc = select_process_SJF();
					break;
			}
			case 2:
			{
				cur_run_proc = select_process_SRT();
				break;
			}
			case 3:
			{
				cur_run_proc = select_process_RR();
				break;
			}
			case 4:
			{
				cur_run_proc = select_process_PR();
				break;
			}
		}
		
		if (cur_run_proc == -1) //현재 실행을 할 프로세스가 없습니다. 단순히 CPU시간만 흐릅니다.
		{
			current_cpu_time++;
			continue;
		}
		
		//이제 이번 특정 시간동안의 실행 결과를 연결 리스트에 저장합니다.
		list_add(ready_que[cur_run_proc], '*'); //실행한 프로세스에는 *을 연결합니다.
		
		for (line_index = 0; line_index < line_many; line_index++)
		{
			if (parsed_str_array[line_index].program_id[0] == '#' || line_index == ready_que[cur_run_proc]) //이미 입력한 실행중인 프로세스와 오류들을 제외하고 전부 공백을 하나씩 연결합니다.
				continue;			
			list_add(line_index, ' ');
		}
		current_cpu_time++;
	}
	
	print_result();
}

 /**
 * 주어진 argv를 이용해서 config파일을 오픈하는 함수입니다.
 */
void file_open(char **argv)
{
	char* file_name;
	file_name = strdup(argv[1]);
	
	argv_file = fopen(file_name,"r");

	if (argv_file == NULL)
	{
		fprintf(stderr,"failed to load data file ‘%s’\n",file_name);
		return;
	}

	//먼저 data파일의 줄 갯수를 셉니다.
	read_new_line_letter(argv_file);
	
	//data파일의 내용을 읽고 적절하게 파싱해서 실행하고 스케쥴링할 프로그램의 데이터를 구합니다.
	read_data_file();
	
	//이제 각 알고리즘마다 한번씩 검사합니다.
	printf("\n[SJF]\n");
	process_run(SJF);
	printf("\n[SRT]\n");
	process_run(SRT);
	printf("\n[RR]\n");
	process_run(RR);
	printf("\n[PR]\n");
	process_run(PR);
}




int main (int argc, char **argv)
{
	if (argc <= 1)
	{
		fprintf (stderr, "input file must specified\n");
		return -1;
	}
	
	file_open(argv);

	/* ... */

	return 0;
}
