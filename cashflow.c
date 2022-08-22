/**
 * File              : cashflow.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 13.06.2022
 * Last Modified Date: 22.08.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "cashflow.h"
#include "SQLiteConnect/SQLiteConnect.h"
#include "uuid4/uuid4.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define STR(str, ...) ({char ___str[4096]; sprintf(___str, str, __VA_ARGS__); ___str;})

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
		"taxes INT, "
		"other_expenses INT, "
		"child_cost INT "
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

	char SQL[BUFSIZ];
	sprintf(SQL, 
			"INSERT INTO cashflow "
			"("
			"uuid, "
			"date, "
			"profession, "
			"salary, "
			"taxes, "
			"other_expenses, "
			"child_cost"
			")"
			"VALUES "
			"("
			"'%s', "
			"%ld, "
			"'%s', "
			"%d, "
			"%d, "
			"%d, "
			"%d "
			")", 
			cashflow.uuid,
			cashflow.date,
			cashflow.profession,
			cashflow.salary,
			cashflow.taxes,
			cashflow.other_expenses,
			cashflow.child_cost);

	if (sqlite_connect_execute(SQL, filepath)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}

	if (callback)
		callback(user_data, &cashflow, NULL);
}

void cashflow_get_sql_request(
		char SQL[4096],
		const char * uuid,
		const char * predicate)
{
	sprintf(SQL, 
		                 "SELECT "
	/*uuid*/        	 "uuid"
	/*date*/             ", cashflow.date"
	/*profession*/       ", profession"
	/*salary*/           ", salary"
	/*dividents*/        ", (SELECT IFNULL(SUM(income),0) FROM cashflow_actives WHERE type = %d AND cashflow_uuid = '%s')"
    /*rent*/             ", (SELECT IFNULL(SUM(income),0) FROM cashflow_actives WHERE type = %d AND cashflow_uuid = '%s')"
    /*business*/         ", (SELECT IFNULL(SUM(income),0) FROM cashflow_actives WHERE type = %d AND cashflow_uuid = '%s')"
    /*taxes*/            ", taxes"
    /*mortgage*/         ", (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')"
    /*education_credit*/ ", (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')"
    /*car_credit*/       ", (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')"
    /*creditcard*/       ", (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')" 
    /*some_credits*/     ", (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')"
    /*other_expenses*/   ", other_expenses"
    /*child_cost*/       ", child_cost" 
    /*children_expenses*/", (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')"
    /*bank_credit*/      ", (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')"
    /*child_count*/      ", (SELECT COUNT(uuid) FROM cashflow_passives WHERE type = %d AND cashflow_uuid = '%s')"

	/*passive_income*/   ", (SELECT IFNULL(SUM(income),0) FROM cashflow_actives WHERE cashflow_uuid = '%s')"

	/*total_income*/    ", (salary + (SELECT IFNULL(SUM(income),0) FROM cashflow_actives WHERE cashflow_uuid = '%s'))"

    /*total_expenses*/   ", taxes + other_expenses + (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE cashflow_uuid = '%s')"

    /*cashflow*/         ", (salary + (SELECT IFNULL(SUM(income),0) FROM cashflow_actives WHERE cashflow_uuid = '%s')) - (taxes + other_expenses + (SELECT IFNULL(SUM(expenses),0) FROM cashflow_passives WHERE cashflow_uuid = '%s'))"

						 " FROM cashflow %s;"
						 ,CA_STOCS, uuid
						 ,CA_PROPERTY, uuid	
						 ,CA_BUSINESS, uuid	
						 ,CP_MORTGAGE, uuid	
						 ,CP_EDUCATION_CREDIT, uuid	
						 ,CP_CAR_CREDIT, uuid	
						 ,CP_CREDIT_CARD, uuid	
						 ,CP_SOME_CREDIT, uuid	
						 ,CP_CHILD, uuid	
						 ,CP_BANK_CREDIT, uuid	
						 ,CP_CHILD, uuid	
						 ,uuid
						 ,uuid
						 ,uuid
						 ,uuid, uuid
			
						 ,predicate
	);
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
			case 0:  strcpy(item.uuid, buff)             ; break; //uuid
			case 1:  item.date              = atoi(buff) ; break; //date
			case 2:  strcpy(item.profession, buff)       ; break; //profession 
			case 3:  item.salary            = atoi(buff) ; break; //salary
			case 4:  item.dividents         = atoi(buff) ; break; //dividents
			case 5:  item.rent              = atoi(buff) ; break; //rent
			case 6:  item.business          = atoi(buff) ; break; //business
			case 7:  item.taxes             = atoi(buff) ; break; //taxes
			case 8:  item.mortgage          = atoi(buff) ; break; //mortgage
			case 9:  item.education_credit  = atoi(buff) ; break; //education_credit
			case 10: item.car_credit        = atoi(buff) ; break; //car_credit
			case 11: item.creditcard        = atoi(buff) ; break; //creditcard
			case 12: item.some_credits      = atoi(buff) ; break; //some_credits
			case 13: item.other_expenses    = atoi(buff) ; break; //other_expenses
			case 14: item.child_cost        = atoi(buff) ; break; //child_cost
			case 15: item.children_expenses = atoi(buff) ; break; //children_expenses
			case 16: item.bank_credit       = atoi(buff) ; break; //bank_credit
			case 17: item.child_count       = atoi(buff) ; break; //child_count
			case 18: item.passive_income    = atoi(buff) ; break; //passive_income
			case 19: item.total_income      = atoi(buff) ; break; //total_income
			case 20: item.total_expenses    = atoi(buff) ; break; //total_expenses
			case 21: item.cashflow          = atoi(buff) ; break; //cashflow
                                                                  
			default:                                       break;
		}
	}

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

	char SQL[BUFSIZ];
	if (predicate) {
		sprintf(SQL, "SELECT * FROM cashflow %s", predicate);	
	} else {
	   	sprintf(SQL, "SELECT * FROM cashflow");
	}

	struct cashflow_for_each_data t = {
		.user_data = user_data,
		.callback = callback
	};
	
	if (sqlite_connect_execute_function(SQL, filepath, &t, cashflow_for_each_callback)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}
}

void 
cashflow_get(
		const char * filepath,
		const char * uuid,
		const char * predicate,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_t * cashflow,
			char * error
			)
		)
{
	if (!uuid){
		if (callback)
			callback(user_data, NULL, "cashflow: cashflow_get: no uuis in request\n");
		return;		
	}

	char SQL[4096];
	if (predicate)
		cashflow_get_sql_request(SQL, uuid, predicate);
	else 
		cashflow_get_sql_request(SQL, uuid, "");

	struct cashflow_for_each_data t = {
		.user_data = user_data,
		.callback = callback
	};

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
	char SQL[BUFSIZ];
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
		sprintf(SQL, "SELECT * FROM cashflow_actives %s", predicate);	
	} else {
	   	sprintf(SQL, "SELECT * FROM cashflow_actives");
	}
	
	if (sqlite_connect_execute_function(SQL, filepath, &t, cashflow_active_for_each_callback)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}	
}

void 
cashflow_active_new(
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
			callback(user_data, NULL, "cashflow: Can't genarate UUID\n");
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
			"income"
			") "
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
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}

	if (callback)
		callback(user_data, &cashflow_active, NULL);
}

int
cashflow_remove_active(
		const char * filepath,
		const char * uuid)
{
	char SQL[BUFSIZ];
	sprintf(SQL, "DELETE FROM cashflow_actives WHERE uuid = '%s'", uuid);

	return sqlite_connect_execute(SQL, filepath);	
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
	sprintf(SQL,	
				"SELECT"
				"  uuid"
				", cashflow_uuid"
				", date"
				", type"
				", ("
				"CASE type "
					"WHEN %d THEN 'Ребёнок' "
					"WHEN %d THEN 'Ипотека' "
					"WHEN %d THEN 'Кредит на образование' "
					"WHEN %d THEN 'Автокредит' "
					"WHEN %d THEN 'Кредитные карты' "
					"WHEN %d THEN 'Прочие кредиты' "
					"WHEN %d THEN 'Банковский кредит' "
				"END) as title"
				", cost"
				", expenses"
				" FROM cashflow_passives "
				,CP_CHILD
				,CP_MORTGAGE
				,CP_EDUCATION_CREDIT
				,CP_CAR_CREDIT
				,CP_CREDIT_CARD
				,CP_SOME_CREDIT
				,CP_BANK_CREDIT
		   );
	
	if (predicate)
		strcat(SQL, predicate);	
	
	if (sqlite_connect_execute_function(SQL, filepath, &t, cashflow_passive_for_each_callback)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}	
}

void 
cashflow_passive_new(
		const char * filepath,
		const char * cashflow_uuid,
		CP_TYPE type,
		int cost,
		int expenses,
		void * user_data,
		int (*callback)(
			void * user_data,
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
			callback(user_data, NULL, "cashflow: Can't genarate UUID\n");
		return;
	}

	//make title
	const char * title;
	switch (type) {
		case CP_CHILD:            title = "Ребёнок"              ; break;
		case CP_MORTGAGE:         title = "Ипотека"              ; break;
		case CP_EDUCATION_CREDIT: title = "Кредит на образование"; break;
		case CP_CAR_CREDIT:       title = "Автокредит"           ; break;
		case CP_CREDIT_CARD:      title = "Кредитные карты"      ; break;
		case CP_SOME_CREDIT:      title = "Прочие кредиты"       ; break;
		case CP_BANK_CREDIT:      title = "Банковский кредит"    ; break;
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
			"cost, "
			"expenses"
			") "
			"VALUES "
			"("
			"'%s', "
			"'%s', "
			"%ld, "
			"%d, "
			"%d, "
			"%d"
			")",
			cashflow_passive.uuid,
			cashflow_passive.cashflow_uuid,
			cashflow_passive.date,
			cashflow_passive.type,
			cashflow_passive.cost,
			cashflow_passive.expenses
			);

	if (sqlite_connect_execute(SQL, filepath)){
		if (callback)
			callback(user_data, NULL, STR("cashflow: Can't execute SQL: %s\n", SQL));
		return;
	}

	if (callback)
		callback(user_data, &cashflow_passive, NULL);

}

struct cashflow_add_child_data {
	char * filepath;
	void * user_data;
	int (*callback)(void * user_data, cashflow_passive_t * cashflow_passive, char * error);
};

int get_child_cost_callback(void * user_data, cashflow_t * cashflow, char * error){
	int * child_cost = user_data;
	*child_cost = cashflow->child_cost;
	return 1; //stop execution
}

void 
cashflow_add_child(
		const char * filepath,
		const char * cashflow_uuid,
		void * user_data,
		int (*callback)(
			void * user_data,
			cashflow_passive_t * cashflow_passive,
			char * error
			)
		)
{
	//get  child cost
	int child_cost = -1;
	cashflow_for_each(filepath, STR("WHERE uuid == '%s'", cashflow_uuid), &child_cost, get_child_cost_callback);

	if (child_cost < 1){
		if (callback)
			callback(user_data, NULL, STR("cashflow: can't get child_cost for uuid: %s", cashflow_uuid));
		return;
	}

	//add passive
	struct cashflow_add_child_data t = {
		.filepath = (char *)filepath,
		.user_data = user_data,
		.callback = callback
	};
	cashflow_passive_new(t.filepath, cashflow_uuid, CP_CHILD, 0, child_cost, t.user_data, t.callback);
}	

int
cashflow_passive_set_value_for_key(
		const char * filepath,
		const char * uuid,
		const char * value,
		const char * key
		)
{
	char SQL[BUFSIZ];
	sprintf(SQL, "UPDATE cashflow_passives SET %s = '%s' WHERE uuid = '%s'", key, value, uuid);
	return sqlite_connect_execute(SQL, filepath);
}

int
cashflow_passive_remove(
		const char * filepath,
		const char * uuid)
{
	char SQL[BUFSIZ];
	sprintf(SQL, "DELETE FROM cashflow_passives WHERE uuid = '%s'", uuid);

	return sqlite_connect_execute(SQL, filepath);	
}
#pragma endregion <CASHFLOW PASSIVE>
