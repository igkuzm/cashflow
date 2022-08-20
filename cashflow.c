/**
 * File              : cashflow.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 13.06.2022
 * Last Modified Date: 20.08.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "cashflow.h"
#include "SQLiteConnect/SQLiteConnect.h"
#include "uuid4/uuid4.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define STR(str, ...) ({char ___str[BUFSIZ]; sprintf(___str, str, __VA_ARGS__); ___str;})

int cashflow_database_init(const char *filepath){
	int res = sqlite_connect_create_database(filepath);	
	if (res)
		return res;
	
	char * SQL = 
		"CREATE TABLE IF NOT EXISTS "
		"cashflow "
		"( "
		"uuid TEXT, "
		"date INT, "
		"profession TEXT, "
		"salary INT, "
		"dividents INT, "
		"rent INT, "
		"business INT, "
		"taxes INT, "
		"mortgage INT, "
		"education_credit INT, "
		"car_credit INT, "
		"creditcard INT, "
		"some_credits INT, "
		"other_expenses INT, "
		"child_cost INT, "
		"children_expenses INT, "
		"bank_credit INT "
		")"
		;
	
	res = sqlite_connect_execute(SQL, filepath);
	if (res)
		return res;

	SQL = 
		"CREATE TABLE IF NOT EXISTS "
		"cashflow_actives "
		"( "
		"uuid TEXT, "
		"cashflow_uuid TEXT, "
		"date INT, "
		"type INT, "
		"title TEXT, "
		"downpayment INT, "
		"cost INT, "
		"income INT "
		")"
		;
	
	res = sqlite_connect_execute(SQL, filepath);
	if (res)
		return res;	

	SQL = 
		"CREATE TABLE IF NOT EXISTS "
		"cashflow_passives "
		"( "
		"uuid TEXT, "
		"cashflow_uuid TEXT, "
		"date INT, "
		"type INT, "
		"title TEXT, "
		"cost INT, "
		"expenses INT "
		")"
		;
	
	res = sqlite_connect_execute(SQL, filepath);
	if (res)
		return res;		

	return 0;
}

void cashflow_new(
		const char * filepath,
		char profession[128],
		int	salary,
		int taxes,
		int child_cost,
		int other_expenses,		
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			char * error
			)
		)
{
	//create uuid
	char uuid[37];
	UUID4_STATE_T state; UUID4_T identifier;
	uuid4_seed(&state);
	uuid4_gen(&state, &identifier);
	if (!uuid4_to_s(identifier, uuid, 37)){
		if (callback)
			callback(user_data, NULL, "cashflow: Can't genarate UUID\n");
		return;
	}
	
	cashflow_t cashflow = {
		.profession[0] = '\0',
		.salary = salary,
		.dividents = 0,
		.rent = 0,
		.business = 0,
		.taxes = taxes,
		.mortgage = 0,
		.education_credit = 0,
		.car_credit = 0,
		.creditcard = 0,
		.some_credits = 0,
		.other_expenses = other_expenses,
		.child_cost = child_cost,
		.children_expenses = 0,
		.bank_credit = 0,
		.total_income = 0,
		.passive_income = 0,
		.total_expenses = 0,
		.cashflow = 0	
	};
	strcpy(cashflow.profession, profession);

	cashflow.date = time(NULL);
	strcpy(cashflow.uuid, uuid);

	char SQL[2 * BUFSIZ];
	sprintf(SQL, 
			"INSERT INTO cashflow "
			"("
			"uuid, "
			"date, "
			"profession, "
			"salary, "
			"dividents, "
			"rent, "
			"business, "
			"taxes, "
			"mortgage, "
			"education_credit, "
			"car_credit, "
			"creditcard, "
			"some_credits, "
			"other_expenses, "
			"child_cost, "
			"children_expenses, "
			"bank_credit "
			")"
			"VALUES "
			"("
			"'%s', "
			"%ld, "
			"'%s', "
			"%d, "
			"0, 0, 0, "
			"%d, "
			"0, 0, 0, 0, 0, "
			"%d, "
			"%d, "
			" 0, 0)", 
			cashflow.uuid,
			cashflow.date,
			cashflow.profession,
			cashflow.salary,
			cashflow.taxes,
			cashflow.other_expenses,
			cashflow.child_cost);

	//log
	callback(user_data, NULL, STR("SQL: %s\n", SQL));

	if (sqlite_connect_execute(SQL, filepath)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}

	if (callback)
		callback(user_data, &cashflow, NULL);
}

struct cashflow_for_each_data {
	void * user_data;
	int (*callback)(void * user_data, cashflow_t * cashflow, char * error);
};

int cashflow_for_each_callback(void *user_data, int argc, char *argv[], char *titles[]){
	struct cashflow_for_each_data *t = user_data;
	struct cashflow_t item;

	for (int i = 0; i < argc; ++i) {
		char buff[128];
		if (!argv[i]) buff[0] = '\0'; //no seg falt on null
		else {
			strncpy(buff, argv[i], 127);
			buff[127] = '\0';
		}

		switch (i) {
			case 0:  strcpy(item.uuid, buff)             ; break;
			case 1:  item.date = atoi(buff)              ; break;
			case 2:  strcpy(item.profession, buff)       ; break;
			case 3:  item.salary = atoi(buff)            ; break;
			case 4:  item.dividents = atoi(buff)		 ; break;
			case 5:  item.rent = atoi(buff)				 ; break;
			case 6:  item.business = atoi(buff)			 ; break;
			case 7:  item.taxes = atoi(buff)			 ; break;
			case 8:  item.mortgage = atoi(buff)			 ; break;
			case 9:  item.education_credit = atoi(buff)  ; break;
			case 10: item.car_credit = atoi(buff)		 ; break;
			case 11: item.some_credits = atoi(buff)		 ; break;
			case 12: item.other_expenses = atoi(buff)	 ; break;
			case 13: item.child_cost = atoi(buff)		 ; break;
			case 14: item.children_expenses = atoi(buff) ; break;
			case 15: item.bank_credit = atoi(buff)		 ; break;

			default:                                       break;
		}
	}

	item.passive_income = 
		item.dividents + item.rent + item.business;

	item.total_income = item.salary + item.passive_income;

	item.total_expenses =
		item.taxes + item.mortgage + item.education_credit + item.car_credit +
		item.creditcard + item.some_credits + item.other_expenses + item.children_expenses +
		item.bank_credit;

	item.cashflow = item.total_income = item.total_expenses;

	if (t->callback)
		return t->callback(t->user_data, &item, NULL);

	return 0;
}

void 
cashflow_for_each(
		const char * filepath,
		const char * predicate,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			char * error
			)
		)
{
	struct cashflow_for_each_data t = {
		.user_data = user_data,
		.callback = callback
	};

	char SQL[BUFSIZ];
	if (predicate) {
		sprintf(SQL, "SELECT * FROM cashflow WHERE %s", predicate);	
	} else {
	   	sprintf(SQL, "SELECT * FROM cashflow");
	}
	
	if (sqlite_connect_execute_function(SQL, filepath, &t, cashflow_for_each_callback)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}
}

int
cashflow_set_value_for_key(
		const char * filepath,
		const char * uuid,
		const char * value,
		const char * key
		)
{
	char SQL[BUFSIZ];
	sprintf(SQL, "UPDATE cashflow SET %s = '%s' WHERE uuid = '%s'", key, value, uuid);
	return sqlite_connect_execute(SQL, filepath);
}

int
cashflow_set_values_for_keys(
		const char * filepath,
		const char * uuid,
		...
		)
{
	char SQL[BUFSIZ*BUFSIZ];
	sprintf(SQL, "UPDATE cashflow SET ");

	va_list args;
	va_start(args, uuid);	
	const char *key	  = va_arg(args, const char*);
	const char *value = va_arg(args, const char*);
	sprintf(SQL, "%s%s = '%s'", SQL, key, value);
	while (key != NULL || value != NULL) {
		key   = va_arg(args, const char*);
		value = va_arg(args, const char*);	
		sprintf(SQL, "%s, %s = '%s'", SQL, key, value);
	}
	va_end(args);
		
	sprintf(SQL, "%s WHERE uuid = '%s'", SQL, uuid);
	sqlite_connect_execute(SQL, filepath);

	return 0;
}

int
cashflow_remove(
		const char * filepath,
		const char * uuid
		)
{
	char SQL[BUFSIZ];
	sprintf(SQL, "DELETE FROM cashflow WHERE uuid = '%s'", uuid);

	return sqlite_connect_execute(SQL, filepath);	
}

#pragma region <CASHFLOW ACTIVE>
struct cashflow_active_for_each_data {
	void * user_data;
	int (*callback)(void * user_data, cashflow_active_t * cashflow_active, char * error);
};

int cashflow_active_for_each_callback(void *user_data, int argc, char *argv[], char *titles[]){
	struct cashflow_active_for_each_data *t = user_data;
	struct cashflow_active_t item;

	for (int i = 0; i < argc; ++i) {
		char buff[128];
		if (!argv[i]) buff[0] = '\0'; //no seg falt on null
		else {
			strncpy(buff, argv[i], 127);
			buff[127] = '\0';
		}

		switch (i) {
			case 0:  strcpy(item.uuid, buff)             ; break;
			case 1:  strcpy(item.cashflow_uuid, buff)    ; break;
			case 2:  item.date = atoi(buff)              ; break;
			case 3:  item.type = atoi(buff)              ; break;
			case 4:  strcpy(item.title, buff)            ; break;
			case 5:  item.downpayment = atoi(buff)		 ; break;
			case 6:  item.cost = atoi(buff)			     ; break;
			case 7:  item.income = atoi(buff)			 ; break;

			default:                                       break;
		}
	}

	if (t->callback)
		return t->callback(t->user_data, &item, NULL);

	return 0;
}
void 
cashflow_active_for_each(
		const char * filepath,
		const char * predicate,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_active_t * cashflow_active,
			char * error
			)
		)
{
	struct cashflow_active_for_each_data t = {
		.user_data = user_data,
		.callback = callback
	};

	char SQL[BUFSIZ];
	if (predicate) {
		sprintf(SQL, "SELECT * FROM cashflow_actives WHERE %s", predicate);	
	} else {
	   	sprintf(SQL, "SELECT * FROM cashflow_actives");
	}
	
	if (sqlite_connect_execute_function(SQL, filepath, &t, cashflow_active_for_each_callback)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}	
}

struct cashflow_add_active_data {
	void * user_data;
	char * filepath;
	cashflow_active_t * cashflow_active;
	int (*callback)(void * user_data, cashflow_t * cashflow, cashflow_active_t * cashflow_active, char * error);
};

int cashflow_add_active_callback(void * user_data, cashflow_t * cashflow, char * error){
	struct cashflow_add_active_data *t = user_data;
	cashflow_active_t * cashflow_active = t->cashflow_active;

	cashflow->passive_income += cashflow_active->income;
	cashflow->total_income += cashflow_active->income;
	cashflow->cashflow += cashflow_active->income;

	char * key;
	int value;
	
	switch (cashflow_active->type) {
		case CA_STOCS: 
			{
				cashflow->dividents += cashflow_active->income;
				key = "dividents";
				value = cashflow->dividents;
				break;
			}
		case CA_PROPERTY: 
			{
				cashflow->rent += cashflow_active->income;
				key = "rent";
				value = cashflow->rent;
				break;
			}
		case CA_BUSINESS:
			{
				cashflow->business += cashflow_active->income;
				key = "business";
				value = cashflow->business;
				break;
			}
		default: break;
	}
	
	char SQL[BUFSIZ];
	sprintf(SQL, "UPDATE cashflow SET %s = %d WHERE uuid = '%s'", key, value, cashflow->uuid);
	int res = sqlite_connect_execute(SQL, t->filepath);
	if (res){
		if (t->callback)
			t->callback(user_data, NULL, NULL, STR("cashflow: Can't execute SQL: %s. Error code: %d\n", SQL, res));
		return 1;
	}
	
	if (t->callback)
		t->callback(user_data, cashflow, cashflow_active, NULL);
	
	return 1; //stop execute
}

void 
cashflow_add_active(
		const char * filepath,
		const char * cashflow_uuid,
		CA_TYPE type,
		char title[128],
		int downpayment,
		int cost,
		int income,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			cashflow_active_t * cashflow_active,
			char * error
			)
		)
{
	//create uuid
	char uuid[37];
	UUID4_STATE_T state; UUID4_T identifier;
	uuid4_seed(&state);
	uuid4_gen(&state, &identifier);
	if (!uuid4_to_s(identifier, uuid, 37)){
		if (callback)
			callback(user_data, NULL, NULL, "cashflow: Can't genarate UUID\n");
		return;
	}
	
	cashflow_active_t cashflow_active = {
		.downpayment = downpayment,
		.cost = cost,
		.income = income
	};
	cashflow_active.date = time(NULL);
	strcpy(cashflow_active.uuid, uuid);
	strcpy(cashflow_active.cashflow_uuid, cashflow_uuid);
	cashflow_active.type = type;
	strcpy(cashflow_active.title, title);

	char SQL[BUFSIZ];
	sprintf(SQL, 
			"INSERT INTO cashflow_actives "
			"("
			"uuid, "
			"cashflow_uuid, "
			"date, "
			"type, "
			"title, "
			"downpayment, "
			"cost, "
			"income "
			")"
			"VALUES "
			"("
			"'%s', "
			"'%s', "
			"%ld, "
			"%d, "
			"'%s', "
			"%d, "
			"%d, "
			"%d "
			")",
			cashflow_active.uuid,
			cashflow_active.cashflow_uuid,
			cashflow_active.date,
			cashflow_active.type,
			cashflow_active.title,
			cashflow_active.downpayment,
			cashflow_active.cost,
			cashflow_active.income
			);

	if (sqlite_connect_execute(SQL, filepath)){
		if (callback)
			callback(user_data, NULL, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}

	struct cashflow_add_active_data t = {
		.user_data = user_data,
		.callback = callback,
		.filepath = (char *)filepath,
		.cashflow_active = &cashflow_active
	};

	cashflow_for_each(filepath, STR("uuid == '%s'", cashflow_uuid), &t, cashflow_add_active_callback);
}

struct cashflow_remove_active_data {
	void * user_data;
	char * filepath;
	cashflow_active_t * cashflow_active;
	int (*callback)(void * user_data, cashflow_t * cashflow, char * error);
};

int cashflow_remove_active_callback(void * user_data, cashflow_t * cashflow, char * error){
	struct cashflow_remove_active_data *t = user_data;
	cashflow_active_t * cashflow_active = t->cashflow_active;

	cashflow->passive_income -= cashflow_active->income;
	cashflow->total_income -= cashflow_active->income;
	cashflow->cashflow -= cashflow_active->income;

	char * key;
	int value;
	
	switch (cashflow_active->type) {
		case CA_STOCS: 
			{
				cashflow->dividents -= cashflow_active->income;
				key = "dividents";
				value = cashflow->dividents;
				break;
			}
		case CA_PROPERTY: 
			{
				cashflow->rent -= cashflow_active->income;
				key = "rent";
				value = cashflow->rent;
				break;
			}
		case CA_BUSINESS:
			{
				cashflow->business -= cashflow_active->income;
				key = "business";
				value = cashflow->business;
				break;
			}
		default: break;
	}
	
	char SQL[BUFSIZ];
	sprintf(SQL, "UPDATE cashflow SET %s = %d WHERE uuid = '%s'", key, value, cashflow->uuid);
	int res;
	res = sqlite_connect_execute(SQL, t->filepath);
	if (res){
		if (t->callback)
			t->callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s. Error code: %d\n", SQL, res));
		return 1;
	}

	sprintf(SQL, "DELETE FROM cashflow_actives WHERE uuid = '%s'", cashflow_active->uuid);	
	res = sqlite_connect_execute(SQL, t->filepath); 
	if (res){
		if (t->callback)
			t->callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s. Error code: %d\n", SQL, res));
		return 1;
	}
	
	if (t->callback)
		t->callback(user_data, cashflow, NULL);
	
	return 1; //stop execute
}

int cashflow_remove_active_callback_get_active(void * user_data, cashflow_active_t * cashflow_active, char * error){
	struct cashflow_remove_active_data *t = user_data;
	t->cashflow_active = cashflow_active; 
	
	cashflow_for_each(t->filepath, STR("uuid == '%s'", cashflow_active->cashflow_uuid), t, cashflow_remove_active_callback);
	return 1; //stop execute
}

void
cashflow_remove_active(
		const char * filepath,
		const char * uuid,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			char * error
			)		
		)
{
	struct cashflow_remove_active_data t = {
		.user_data = user_data,
		.callback = callback,
		.filepath = (char *)filepath,
	};
	
	cashflow_active_for_each(filepath, STR("uuid == '%s'", uuid), &t, cashflow_remove_active_callback_get_active);
}
#pragma endregion <CASHFLOW ACTIVE>
#pragma region <CASHFLOW PASSIVE>
struct cashflow_passive_for_each_data {
	void * user_data;
	int (*callback)(void * user_data, cashflow_passive_t * cashflow_passive, char * error);
};

int cashflow_passive_for_each_callback(void *user_data, int argc, char *argv[], char *titles[]){
	struct cashflow_passive_for_each_data *t = user_data;
	struct cashflow_passive_t item;

	for (int i = 0; i < argc; ++i) {
		char buff[128];
		if (!argv[i]) buff[0] = '\0'; //no seg falt on null
		else {
			strncpy(buff, argv[i], 127);
			buff[127] = '\0';
		}

		switch (i) {
			case 0:  strcpy(item.uuid, buff)             ; break;
			case 1:  strcpy(item.cashflow_uuid, buff)    ; break;
			case 2:  item.date = atoi(buff)              ; break;
			case 3:  item.type = atoi(buff)              ; break;
			case 4:  strcpy(item.title, buff)            ; break;
			case 5:  item.cost = atoi(buff)			     ; break;
			case 6:  item.expenses = atoi(buff)			 ; break;

			default:                                       break;
		}
	}

	if (t->callback)
		return t->callback(t->user_data, &item, NULL);

	return 0;
}
void 
cashflow_passive_for_each(
		const char * filepath,
		const char * predicate,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_passive_t * cashflow_passive,
			char * error
			)
		)
{
	struct cashflow_passive_for_each_data t = {
		.user_data = user_data,
		.callback = callback
	};

	char SQL[BUFSIZ];
	if (predicate) {
		sprintf(SQL, "SELECT * FROM cashflow_passives WHERE %s", predicate);	
	} else {
	   	sprintf(SQL, "SELECT * FROM cashflow_passives");
	}
	
	if (sqlite_connect_execute_function(SQL, filepath, &t, cashflow_passive_for_each_callback)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}	
}

struct cashflow_add_passive_data {
	void * user_data;
	char * filepath;
	cashflow_passive_t * cashflow_passive;
	int (*callback)(void * user_data, cashflow_t * cashflow, cashflow_passive_t * cashflow_passive, char * error);
};

int cashflow_add_passive_callback(void * user_data, cashflow_t * cashflow, char * error){
	struct cashflow_add_passive_data *t = user_data;
	cashflow_passive_t * cashflow_passive = t->cashflow_passive;

	cashflow->total_expenses += cashflow_passive->expenses;
	cashflow->cashflow -= cashflow_passive->expenses;

	char * key;
	int value;
	
	switch (cashflow_passive->type) {
		case CP_CHILD: 
			{
				cashflow->children_expenses += cashflow_passive->expenses;
				key = "children_expenses";
				value = cashflow->children_expenses;
				break;
			}
		case CP_MORTGAGE: 
			{
				cashflow->mortgage += cashflow_passive->expenses;
				key = "mortgage";
				value = cashflow->mortgage;
				break;
			}
		case CP_EDUCATION_CREDIT:
			{
				cashflow->education_credit += cashflow_passive->expenses;
				key = "education_credit";
				value = cashflow->education_credit;
				break;
			}
		case CP_CAR_CREDIT:
			{
				cashflow->car_credit += cashflow_passive->expenses;
				key = "car_credit";
				value = cashflow->car_credit;
				break;
			}			
		case CP_CREDIT_CARD:
			{
				cashflow->creditcard += cashflow_passive->expenses;
				key = "creditcard";
				value = cashflow->creditcard;
				break;
			}
		case CP_SOME_CREDIT:
			{
				cashflow->some_credits += cashflow_passive->expenses;
				key = "some_credits";
				value = cashflow->some_credits;
				break;
			}	
		case CP_BANK_CREDIT:
			{
				cashflow->bank_credit += cashflow_passive->expenses;
				key = "bank_credit";
				value = cashflow->bank_credit;
				break;
			}	
		case CP_BUSINESS:
			{
				cashflow->business -= cashflow_passive->expenses;
				key = "business";
				value = cashflow->business;
				break;
			}			
		default: break;
	}
	
	char SQL[BUFSIZ];
	sprintf(SQL, "UPDATE cashflow SET %s = %d WHERE uuid = '%s'", key, value, cashflow->uuid);
	if (sqlite_connect_execute(SQL, t->filepath)){
		if (t->callback)
			t->callback(user_data, NULL, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return 1;
	}
	
	if (t->callback)
		t->callback(user_data, cashflow, cashflow_passive, NULL);
	
	return 1; //stop execute
}

void 
cashflow_add_passive(
		const char * filepath,
		const char * cashflow_uuid,
		CP_TYPE type,
		char title[128],
		int cost,
		int expenses,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			cashflow_passive_t * cashflow_passive,
			char * error
			)
		)
{
	//create uuid
	char uuid[37];
	UUID4_STATE_T state; UUID4_T identifier;
	uuid4_seed(&state);
	uuid4_gen(&state, &identifier);
	if (!uuid4_to_s(identifier, uuid, 37)){
		if (callback)
			callback(user_data, NULL, NULL, "cashflow: Can't genarate UUID\n");
		return;
	}
	
	cashflow_passive_t cashflow_passive = {
		.cost = cost,
		.expenses = expenses
	};
	cashflow_passive.date = time(NULL);
	strcpy(cashflow_passive.uuid, uuid);
	strcpy(cashflow_passive.cashflow_uuid, cashflow_uuid);
	cashflow_passive.type = type;
	strcpy(cashflow_passive.title, title);

	char SQL[BUFSIZ];
	sprintf(SQL, 
			"INSERT INTO cashflow_passives "
			"("
			"uuid, "
			"cashflow_uuid, "
			"date, "
			"type, "
			"title, "
			"cost, "
			"expenses "
			")"
			"VALUES "
			"("
			"'%s', "
			"'%s', "
			"%ld, "
			"%d, "
			"'%s'', "
			"%d, "
			"%d "
			")",
			cashflow_passive.uuid,
			cashflow_passive.cashflow_uuid,
			cashflow_passive.date,
			cashflow_passive.type,
			cashflow_passive.title,
			cashflow_passive.cost,
			cashflow_passive.expenses
			);

	if (sqlite_connect_execute(SQL, filepath)){
		if (callback)
			callback(user_data, NULL, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}

	struct cashflow_add_passive_data t = {
		.user_data = user_data,
		.callback = callback,
		.filepath = (char *)filepath,
		.cashflow_passive = &cashflow_passive
	};

	cashflow_for_each(filepath, STR("uuid == '%s'", cashflow_uuid), &t, cashflow_add_passive_callback);
}

struct cashflow_add_child_data {
	char * filepath;
	void * user_data;
	int (*callback)(void * user_data, cashflow_t * cashflow, cashflow_passive_t * cashflow_passive, char * error);
};

int cashflow_add_child_callback(void * user_data, cashflow_t * cashflow, char * error){
	struct cashflow_add_child_data *t = user_data;
	cashflow_add_passive(t->filepath, cashflow->uuid, CP_CHILD, "child", 0, cashflow->child_cost, t->user_data, t->callback);
	return 1; //stop execution
}

void 
cashflow_add_child(
		const char * filepath,
		const char * cashflow_uuid,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			cashflow_passive_t * cashflow_passive,
			char * error
			)
		)
{
	struct cashflow_add_child_data t = {
		.filepath = (char *)filepath,
		.user_data = user_data,
		.callback = callback
	};

	cashflow_for_each(filepath, STR("uuid == '%s'", cashflow_uuid), &t, cashflow_add_child_callback);
}	


struct cashflow_remove_passive_data {
	void * user_data;
	char * filepath;
	cashflow_passive_t * cashflow_passive;
	int (*callback)(void * user_data, cashflow_t * cashflow, char * error);
};

int cashflow_remove_passive_callback(void * user_data, cashflow_t * cashflow, char * error){
	struct cashflow_remove_passive_data *t = user_data;
	cashflow_passive_t * cashflow_passive = t->cashflow_passive;

	cashflow->total_expenses -= cashflow_passive->expenses;
	cashflow->cashflow += cashflow_passive->expenses;

	char * key;
	int value;
	
	switch (cashflow_passive->type) {
		case CP_CHILD: 
			{
				cashflow->children_expenses -= cashflow_passive->expenses;
				key = "children_expenses";
				value = cashflow->children_expenses;
				break;
			}
		case CP_MORTGAGE: 
			{
				cashflow->mortgage -= cashflow_passive->expenses;
				key = "mortgage";
				value = cashflow->mortgage;
				break;
			}
		case CP_EDUCATION_CREDIT:
			{
				cashflow->education_credit -= cashflow_passive->expenses;
				key = "education_credit";
				value = cashflow->education_credit;
				break;
			}
		case CP_CAR_CREDIT:
			{
				cashflow->car_credit -= cashflow_passive->expenses;
				key = "car_credit";
				value = cashflow->car_credit;
				break;
			}			
		case CP_CREDIT_CARD:
			{
				cashflow->creditcard -= cashflow_passive->expenses;
				key = "creditcard";
				value = cashflow->creditcard;
				break;
			}
		case CP_SOME_CREDIT:
			{
				cashflow->some_credits -= cashflow_passive->expenses;
				key = "some_credits";
				value = cashflow->some_credits;
				break;
			}	
		case CP_BANK_CREDIT:
			{
				cashflow->bank_credit -= cashflow_passive->expenses;
				key = "bank_credit";
				value = cashflow->bank_credit;
				break;
			}	
		case CP_BUSINESS:
			{
				cashflow->business += cashflow_passive->expenses;
				key = "business";
				value = cashflow->business;
				break;
			}			
		default: break;
	}
	
	char SQL[BUFSIZ];
	sprintf(SQL, "UPDATE cashflow SET %s = %d WHERE uuid = '%s'", key, value, cashflow->uuid);
	if (sqlite_connect_execute(SQL, t->filepath)){
		if (t->callback)
			t->callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return 1;
	}

	sprintf(SQL, "DELETE FROM cashflow_passives WHERE uuid = '%s'", cashflow_passive->uuid);	
	if (sqlite_connect_execute(SQL, t->filepath)){
		if (t->callback)
			t->callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return 1;
	}
	
	if (t->callback)
		t->callback(user_data, cashflow, NULL);
	
	return 1; //stop execute
}

int cashflow_remove_passive_callback_get_passive(void * user_data, cashflow_passive_t * cashflow_passive, char * error){
	struct cashflow_remove_passive_data *t = user_data;
	t->cashflow_passive = cashflow_passive; 
	
	cashflow_for_each(t->filepath, STR("uuid == '%s'", cashflow_passive->cashflow_uuid), t, cashflow_remove_passive_callback);
	return 1; //stop execute
}

void
cashflow_remove_passive(
		const char * filepath,
		const char * uuid,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			char * error
			)		
		)
{
	struct cashflow_remove_passive_data t = {
		.user_data = user_data,
		.callback = callback,
		.filepath = (char *)filepath,
	};
	
	cashflow_passive_for_each(filepath, STR("uuid == '%s'", uuid), &t, cashflow_remove_passive_callback_get_passive);
}
#pragma endregion <CASHFLOW PASSIVE>



