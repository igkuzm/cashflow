/**
 * File              : cashflow.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 13.06.2022
 * Last Modified Date: 14.06.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef cashflow_h__
#define cashflow_h__

#ifdef __cplusplus
extern "C"{
#endif

	#include <time.h>
/**
 * This library to craft usable application to play cashflow game
 */

	typedef struct cashflow_t {
		char uuid[37];
		time_t date;
		char profession[128];		//profession in game
		int	salary;					//salary in usa dollars
		int dividents;				//income from dividents in usa dollars
		int rent;					//income from rent
		int business;				//income from business
		int taxes;					//expenses of taxes
		int mortgage;				//hypotec expenses
		int education_credit;		//education credit expenses
		int car_credit;				//automobile credit expenses
		int creditcard;				//credit card expenses
		int some_credits;			//credits expenses
		int other_expenses;			//other expenses
		int child_cost;				//expenses of one child
		int children_expenses;		//children expenses
		int bank_credit;			//bank credit expenses
		int passive_income;
		int total_income;
		int total_expenses;
		int cashflow;
	}cashflow_t;

	/*! \enum cashflow_actives
	 *
	 *  The types of actives
	 */
	typedef enum cashflow_active_type { 
		CA_STOCS,
		CA_PROPERTY,
		CA_BUSINESS	
	}CA_TYPE;

	typedef struct cashflow_active_t {
		char uuid[37];
		char cashflow_uuid[37];
		time_t date;
		CA_TYPE type;
		char title[128];
		int downpayment;
		int cost;
		int income;
	}cashflow_active_t;

	typedef enum cashflow_passive_type { 
		CP_CHILD,
		CP_MORTGAGE,
		CP_EDUCATION_CREDIT,
		CP_CAR_CREDIT,
		CP_CREDIT_CARD,
		CP_SOME_CREDIT,
		CP_BANK_CREDIT,
		CP_BUSINESS	
	}CP_TYPE;	

	typedef struct cashflow_passive_t {
		char uuid[37];
		char cashflow_uuid[37];
		time_t date;		
		CP_TYPE type;
		char title[128];
		int cost;
		int expenses;
	} cashflow_passive_t;

	//create (if not exists) SQLite databese in filepath and init its structure
	int cashflow_database_init(const char * filepath);

	//create new cashflow game player
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
			);

	//execute callback for each cashflow
	void cashflow_for_each(
			const char * filepath,
			const char * predicate,
			void * user_data,
			int (*callback)(
				void * user_data,
				cashflow_t * cashflow,
				char * error
				)
			);

	//set value for key
	int cashflow_set_value_for_key(
			const char * filepath,
			const char * uuid,
			const char * value,
			const char * key
			);	

	int cashflow_set_values_for_keys(
			const char * filepath,
			const char * uuid,
			...
			);

	//remove
	int cashflow_remove(
			const char * filepath,
			const char * uuid
			);

	
	//add new cashflow active
	void cashflow_add_active(
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
			);	

	//execute callback for each cashflow active
	void cashflow_active_for_each(
			const char * filepath,
			const char * predicate,
			void * user_data,
			int (*callback)(
				void * user_data,
				cashflow_active_t * cashflow_active,
				char * error
				)
			);	

	void cashflow_remove_active(
			const char * filepath,
			const char * uuid,
			void * user_data,
			int (*callback)(
				void * user_data,
				cashflow_t * cashflow,
				char * error
				)
			);	

	//add new cashflow passive
	void cashflow_add_passive(
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
			);	

	void cashflow_add_child(
			const char * filepath,
			const char * cashflow_uuid,
			void * user_data,
			int (*callback)(
				void * user_data,
				cashflow_t * cashflow,
				cashflow_passive_t * cashflow_passive,
				char * error
				)
			);	

	//execute callback for each cashflow passive	
	void cashflow_passive_for_each(
			const char * filepath,
			const char * predicate,
			void * user_data,
			int (*callback)(
				void * user_data,
				cashflow_passive_t * cashflow_passive,
				char * error
				)
			);	
		

#ifdef __cplusplus
}
#endif

#endif //cashflow_h__	

