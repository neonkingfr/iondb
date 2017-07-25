/******************************************************************************/
/**
@file		iinq_user.c
@author		Dana Klamut
@brief		This code contains definitions for iinq user functions
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "iinq_user.h"
#include <string.h>
#include <unistd.h>

void
uppercase(
	char	*string,
	char	uppercase[]
) {
	int i;
	int len = (int) strlen(string);

	for (i = 0; i < len; i++) {
		uppercase[i] = (char) toupper(string[i]);
	}

	uppercase[i] = '\0';
}

char *
next_keyword(
	char *keyword
) {
	if (0 == strncmp("SELECT", keyword, 6)) {
		return "FROM";
	}

	if (0 == strncmp("FROM", keyword, 4)) {
		return "WHERE";
	}

	if (0 == strncmp("WHERE", keyword, 5)) {
		return "ORDERBY";
	}

	if (0 == strncmp("ORDERBY", keyword, 7)) {
		return "GROUPBY";
	}

	if (0 == strncmp("GROUPBY", keyword, 7)) {
		return NULL;
	}
	else {
		return NULL;
	}
}

ion_err_t
get_clause(
	char	*keyword,
	char	sql[],
	char	clause[]
) {
	char	*end_pointer, *start_pointer, *curr_key;
	int		start_pos, end_pos;

	/* Get position of next keyword (if exists) */
	curr_key = next_keyword(keyword);

	while (1) {
		if (NULL != curr_key) {
			end_pointer = strstr(sql, curr_key);

			if (NULL != end_pointer) {
				break;
			}
		}
		else {
			end_pointer = NULL;
			break;
		}

		curr_key = next_keyword(curr_key);
	}

	start_pointer = strstr(sql, keyword);

	if (NULL == start_pointer) {
		return err_uninitialized;
	}

	if (NULL == end_pointer) {
		end_pos		= (int) (strlen(sql) + 1);
		start_pos	= (int) (start_pointer - sql);
	}
	else {
		end_pos		= (int) (end_pointer - sql);
		start_pos	= (int) (start_pointer - sql);
	}

	/* Get clause */
	memcpy(clause, &sql[start_pos], end_pos - 1);
	clause[(end_pos - start_pos) - 1] = '\0';

	return err_ok;
}

void
get_field_list(
	char	*keyword,
	char	clause[],
	char	field_list[]
) {
	memcpy(field_list, &clause[(int) strlen(keyword) + 1], (int) strlen(clause));
}

void
table_cleanup(
) {
	fremove("1.ffs");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("ion_mt.tbl");
	fremove("TEST1.INQ");
	fremove("TEST2.INQ");
	fremove("TEST3.INQ");
}

int
get_int(
	void *value
) {
	return NEUTRALIZE(value, int);
}

void
group(
	ion_dictionary_t	*dictionary,
	ion_record_t		records[3]
) {
	ion_predicate_t predicate;
	int				num_records = 0;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = malloc(sizeof(ion_dict_cursor_t));

	dictionary_find(dictionary, &predicate, &cursor);

	/* Initialize records */
	records[0].key		= malloc((size_t) dictionary->instance->record.key_size);
	records[0].value	= malloc((size_t) dictionary->instance->record.value_size);
	records[1].key		= malloc((size_t) dictionary->instance->record.key_size);
	records[1].value	= malloc((size_t) dictionary->instance->record.value_size);
	records[2].key		= malloc((size_t) dictionary->instance->record.key_size);
	records[2].value	= malloc((size_t) dictionary->instance->record.value_size);

	ion_cursor_status_t cursor_status;

	while (3 > num_records) {
		while ((cursor_status = cursor->next(cursor, &records[num_records])) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
			num_records++;
		}
	}

	int		col_val;
	char	num[4];
	char	*result;

	/* Sort records in array ASC */
	/* Modify for total number of records in table */
	for (int i = 0; i < 3; ++i) {
		for (int j = i + 1; j < 3; ++j) {
			/* If col2 has same same value -> group */
			if ((NULL != records[i].key) && (NULL != records[j].key) && (0 == strncmp((char *) (records[i].value), (char *) (records[j].value), 3))) {
				result	= malloc(strlen(records[i].value) + 1);
				col_val = (atoi(records[i].value + 3)) + (atoi(records[j].value + 3));

				sprintf(num, "%i", col_val);
				num[3]	= '\0';

				memcpy(result, records[i].value, 3);
				memcpy(result + 3, num, 4);

				records[i].value	= result;
				records[j].key		= NULL;
				records[j].value	= NULL;
			}
		}
	}

	cursor->destroy(&cursor);
}

void
sort(
	ion_dictionary_t	*dictionary,
	ion_record_t		records[3],
	ion_boolean_t		ASC,
	ion_boolean_t		grouped
) {
	ion_predicate_t predicate;
	int				num_records = 0;

	if (!grouped) {
		dictionary_build_predicate(&predicate, predicate_all_records);

		ion_dict_cursor_t *cursor = malloc(sizeof(ion_dict_cursor_t));

		dictionary_find(dictionary, &predicate, &cursor);

		/* Initialize records */
		records[0].key		= malloc((size_t) dictionary->instance->record.key_size);
		records[0].value	= malloc((size_t) dictionary->instance->record.value_size);
		records[1].key		= malloc((size_t) dictionary->instance->record.key_size);
		records[1].value	= malloc((size_t) dictionary->instance->record.value_size);
		records[2].key		= malloc((size_t) dictionary->instance->record.key_size);
		records[2].value	= malloc((size_t) dictionary->instance->record.value_size);

		ion_cursor_status_t cursor_status;

		while (3 > num_records) {
			while ((cursor_status = cursor->next(cursor, &records[num_records])) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				num_records++;
			}
		}

		cursor->destroy(&cursor);
	}

	ion_record_t max_key;

	/* Sort records in array ASC */
	/* Modify for total number of records in table */
	if (ASC) {
		for (int i = 0; i < 3; ++i) {
			for (int j = i + 1; j < 3; ++j) {
				if ((NULL != records[i].key) && (NULL != records[j].key) && (get_int(records[i].key) > get_int(records[j].key))) {
					max_key		= records[i];
					records[i]	= records[j];
					records[j]	= max_key;
				}
			}
		}
	}
	/* Sort records in array DESC */
	/* Modify for total number of records in table */
	else {
		for (int i = 0; i < 3; ++i) {
			for (int j = i + 1; j < 3; ++j) {
				if (get_int(records[i].key) < get_int(records[j].key)) {
					max_key		= records[i];
					records[i]	= records[j];
					records[j]	= max_key;
				}
			}
		}
	}
}

ion_record_t
next_record(
	ion_record_t	sorted_records[],
	ion_record_t	record
) {
	int pos = 0;

	while (3 > pos) {
		if ((NULL != sorted_records[pos].key) && (NULL != sorted_records[pos].value)) {
			record					= sorted_records[pos];

			/* Set record to NULL as not to return it repeatedly */
			sorted_records[pos].key = NULL;
			return record;
		}

		pos++;
	}

	/* Array is empty - all records have been previously returned */
	record.key = NULL;
	return record;
}

ion_record_t
next(
	ion_query_iterator_t	*iterator,
	ion_record_t			record
) {
	ion_cursor_status_t cursor_status;

	int sum			= 0;
	int count		= 1;
	int avg_count	= 0;

	/* Set-up for SELECT fieldlist condition */
	if ((iterator->select_fieldlist) && !(iterator->orderby_condition) && !(iterator->groupby_condition) && !(iterator->sum_condition) && !(iterator->avg_condition) && !(iterator->count_condition)) {
		while (((cursor_status = iterator->cursor->next(iterator->cursor, &record)) == cs_cursor_active) || (cursor_status == cs_cursor_initialized)) {
			/* Evaluate WHERE condition */
			if ((iterator->where_condition) && !(iterator->orderby_condition)) {
				if (3 > get_int(record.key)) {
					/* Return the retrieved record that meets WHERE condition */
					record.value = (char *) (record.value + 3);
					return record;
				}
			}
			/* No WHERE or ORDERBY or GROUPBY condition - return any retrieved record */
			else {
				record.value = (char *) (record.value + 3);
				return record;
			}
		}
	}

	/* Evaluate ORDERBY condition */
	if (iterator->orderby_condition) {
		record = next_record(iterator->sorted_records, record);

		while (NULL != (record.key)) {
			/* If WHERE condition, evaluate */
			if (iterator->where_condition && !iterator->minmax_condition) {
				if (3 > get_int(record.key)) {
					if (iterator->select_fieldlist) {
						record.value = (char *) (record.value + 3);
					}

					return record;
				}
			}
			/* If WHERE condition and MIN/MAX condition, evaluate */
			else if (iterator->where_condition && iterator->minmax_condition) {
				return record;
			}
			else {
				/* Evaluate FIELDLIST condition */
				if (iterator->select_fieldlist) {
					record.value = (char *) (record.value + 3);
				}

				return record;
			}

			record = next_record(iterator->sorted_records, record);
		}
	}
	else {
		while (((cursor_status = iterator->cursor->next(iterator->cursor, &record)) == cs_cursor_active) || (cursor_status == cs_cursor_initialized)) {
			/* Evaluate WHERE condition */
			if ((iterator->where_condition) && !(iterator->orderby_condition) && !(iterator->sum_condition) && !(iterator->avg_condition) && !(iterator->count_condition)) {
				if (3 > get_int(record.key)) {
					/* Return the retrieved record that meets WHERE condition */
					return record;
				}
			}
			/* Evaluate SUM or AVG aggregate */
			else if ((iterator->sum_condition) || (iterator->avg_condition) || (iterator->count_condition)) {
				char	num[4];
				char	*result;

				if ((NULL != record.key) &&
					/* Evaluate WHERE condition if it exists */
					((!iterator->where_condition) || (0 == strncmp((char *) record.value, "100", 3)))) {
					result	= malloc(strlen(record.value) + 1);
					sum		+= (atoi(record.value + 3));

					sprintf(num, "%i", sum);
					num[3]	= '\0';

					memcpy(result, record.value, 3);
					memcpy(result + 3, num, 4);

					record.value = result;
					avg_count++;
				}

				count++;

				if (4 == count) {
					/* Evaluate AVG aggregate */
					if (iterator->avg_condition) {
						int avg = 0;

						result	= malloc(strlen(record.value) + 1);
						avg		= (atoi(record.value + 3)) / avg_count;

						sprintf(num, "%i", avg);
						num[3]	= '\0';

						memcpy(result, record.value, 3);
						memcpy(result + 3, num, 4);

						record.value = result;
						return record;
					}

					if (iterator->count_condition) {
						result	= malloc(sizeof(avg_count) + 1);

						sprintf(num, "%i", avg_count);
						num[3]	= '\0';

						memcpy(result, num, 4);

						record.value = result;
						return record;
					}

					/* Return SUM record as aggregate has been evaluated */
					return record;
				}
			}
			else {
				/* Evaluate GROUPBY condition */
				if (iterator->groupby_condition) {
					record = next_record(iterator->grouped_records, record);

					if (NULL != record.key) {
						return record;
					}

					break;
				}

				/* No WHERE or ORDERBY or GROUPBY condition - return any retrieved record */
				if (NULL != record.key) {
					return record;
				}
			}
		}
	}

	record.key		= NULL;
	record.value	= NULL;

	table_cleanup();
	iterator->destroy(iterator);

	return record;
}

void
destroy(
	ion_query_iterator_t *iterator
) {
/*	if(iterator->orderby_condition) { */
/*		free(iterator->sorted_records[0].key); */
/*		free(iterator->sorted_records[0].value); */
/*		free(iterator->sorted_records[1].key); */
/*		free(iterator->sorted_records[1].value); */
/*		free(iterator->sorted_records[2].key); */
/*		free(iterator->sorted_records[2].value); */
/*	} */

	iterator->cursor->destroy(&iterator->cursor);
/*	free(iterator->record.key); */
/*	free(iterator->record.value); */
}

void
print_records(
	char *table
) {
	printf("Records inserted into table:\n");

	if (0 == strncmp("TEST1.INQ", table, 6)) {
		printf("Key: 1 Value: 100\n");
		printf("Key: 3 Value: 300\n");
		printf("Key: 2 Value: 200\n\n");
	}

	if (0 == strncmp("TEST2.INQ", table, 6)) {
		printf("Key: 1 Value: one, two\n");
		printf("Key: 3 Value: thr, for\n");
		printf("Key: 2 Value: fiv, six\n\n");
	}

	if (0 == strncmp("TEST3.INQ", table, 6)) {
		printf("Key: 1 Value: 100, 200\n");
		printf("Key: 3 Value: 111, 150\n");
		printf("Key: 2 Value: 100, 250\n\n");
	}
}

void
table_setup(
) {
	/* Table set-up */
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "TEST1.INQ";
	key_type			= key_type_numeric_signed;
	key_size			= sizeof(int);
	value_size			= sizeof(int);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source(schema_file_name, key_type, key_size, value_size);

	if (err_ok != error) {
		printf("Error1\n");
	}

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error2\n");
	}

	/* Insert values into table */
	ion_key_t	key1	= IONIZE(1, int);
	ion_value_t value1	= IONIZE(100, int);
	ion_key_t	key2	= IONIZE(3, int);
	ion_value_t value2	= IONIZE(300, int);
	ion_key_t	key3	= IONIZE(2, int);
	ion_value_t value3	= IONIZE(200, int);

	ion_status_t status = dictionary_insert(&dictionary, key1, value1);

	if (err_ok != status.error) {
		printf("Error3\n");
	}

	status = dictionary_insert(&dictionary, key2, value2);

	if (err_ok != status.error) {
		printf("Error4\n");
	}

	status = dictionary_insert(&dictionary, key3, value3);

	if (err_ok != status.error) {
		printf("Error5\n");
	}

	/* Close table and clean-up files */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error6\n");
	}
}

void
fieldlist_table_setup(
) {
	/* Table set-up */
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "TEST2.INQ";
	key_type			= key_type_numeric_signed;
	key_size			= sizeof(int);
	value_size			= sizeof("one") + sizeof("two");

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source(schema_file_name, key_type, key_size, value_size);

	if (err_ok != error) {
		printf("Error1\n");
	}

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error2\n");
	}

	/* Insert values into table */
	ion_key_t	key1	= IONIZE(1, int);
	ion_value_t value1	= "onetwo";
	ion_key_t	key2	= IONIZE(3, int);
	ion_value_t value2	= "thrfor";
	ion_key_t	key3	= IONIZE(2, int);
	ion_value_t value3	= "fivsix";

	ion_status_t status = dictionary_insert(&dictionary, key1, value1);

	if (err_ok != status.error) {
		printf("Error3\n");
	}

	status = dictionary_insert(&dictionary, key2, value2);

	if (err_ok != status.error) {
		printf("Error4\n");
	}

	status = dictionary_insert(&dictionary, key3, value3);

	if (err_ok != status.error) {
		printf("Error5\n");
	}

	/* Close table and clean-up files */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error6\n");
	}
}

void
groupby_table_setup(
) {
	/* Table set-up */
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "TEST3.INQ";
	key_type			= key_type_numeric_signed;
	key_size			= sizeof(int);
	value_size			= sizeof("100") + sizeof("200");

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source(schema_file_name, key_type, key_size, value_size);

	if (err_ok != error) {
		printf("Error1\n");
	}

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error2\n");
	}

	/* Insert values into table */
	ion_key_t	key1	= IONIZE(1, int);
	ion_value_t value1	= "100200";
	ion_key_t	key2	= IONIZE(3, int);
	ion_value_t value2	= "111150";
	ion_key_t	key3	= IONIZE(2, int);
	ion_value_t value3	= "100250";

	ion_status_t status = dictionary_insert(&dictionary, key1, value1);

	if (err_ok != status.error) {
		printf("Error3\n");
	}

	status = dictionary_insert(&dictionary, key2, value2);

	if (err_ok != status.error) {
		printf("Error4\n");
	}

	status = dictionary_insert(&dictionary, key3, value3);

	if (err_ok != status.error) {
		printf("Error5\n");
	}

	/* Close table and clean-up files */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error6\n");
	}
}

ion_err_t
SQL_query(
	ion_query_iterator_t	*iterator,
	char					*sql_string
) {
	char uppercase_sql[(int) strlen(sql_string)];

	uppercase(sql_string, uppercase_sql);

	char		select_clause[50];
	ion_err_t	err					= get_clause("SELECT", uppercase_sql, select_clause);
	char		select_fields[40]	= "null";

	iterator->where_condition	= boolean_false;
	iterator->orderby_condition = boolean_false;
	iterator->groupby_condition = boolean_false;
	iterator->select_fieldlist	= boolean_false;
	iterator->orderby_asc		= boolean_false;
	iterator->minmax_condition	= boolean_false;
	iterator->sum_condition		= boolean_false;
	iterator->avg_condition		= boolean_false;
	iterator->count_condition	= boolean_false;

	if (err == err_ok) {
		get_field_list("SELECT", select_clause, select_fields);

		char *min_pointer = strstr(select_fields, "MIN");

		if (NULL != min_pointer) {
			iterator->orderby_asc		= boolean_true;
			iterator->minmax_condition	= boolean_true;
		}

		char *max_pointer = strstr(select_fields, "MAX");

		if (NULL != max_pointer) {
			iterator->minmax_condition = boolean_true;
		}

		char *sum_pointer = strstr(select_fields, "SUM");

		if (NULL != sum_pointer) {
			iterator->sum_condition = boolean_true;
		}

		char *avg_pointer = strstr(select_fields, "AVG");

		if (NULL != avg_pointer) {
			iterator->avg_condition = boolean_true;
		}

		char *count_pointer = strstr(select_fields, "COUNT");

		if (NULL != count_pointer) {
			iterator->count_condition = boolean_true;
		}
	}

	char from_clause[50];

	err = get_clause("FROM", uppercase_sql, from_clause);

	char from_fields[40] = "null";

	if (err == err_ok) {
		get_field_list("FROM", from_clause, from_fields);
	}

	print_records(from_fields);
	printf("%s\n", uppercase_sql);

	char	where_clause[50];
	char	where_fields[40] = "null";

	err = get_clause("WHERE", uppercase_sql, where_clause);

	if (err == err_ok) {
		get_field_list("WHERE", where_clause, where_fields);
		iterator->where_condition = boolean_true;
	}

	/* Evaluate FROM here */
	ion_dictionary_t			*dictionary = malloc(sizeof(ion_dictionary_t));
	ion_dictionary_handler_t	handler;

	/* currently only supports one table */
	char *schema_file_name = from_fields;

	dictionary->handler = &handler;

	ion_err_t error = iinq_open_source(schema_file_name, dictionary, &handler);

	if (err_ok != error) {
		printf("Error7\n");
	}

	iterator->schema_file_name = from_fields;

	/* Set-up GROUPBY clause if exists */
	char	groupby_clause[50];
	char	groupby_fields[40] = "null";

	err = get_clause("GROUPBY", uppercase_sql, groupby_clause);

	if (err == err_ok) {
		get_field_list("GROUPBY", groupby_clause, groupby_fields);
		iterator->groupby_condition = boolean_true;

		ion_record_t grouped_records[3];

		group(dictionary, grouped_records);

		iterator->grouped_records[0]	= grouped_records[0];
		iterator->grouped_records[1]	= grouped_records[1];
		iterator->grouped_records[2]	= grouped_records[2];
	}

	/* Set-up ORDERBY clause if exists */
	char	orderby_clause[50];
	char	orderby_fields[40] = "null";

	err = get_clause("ORDERBY", uppercase_sql, orderby_clause);

	if (iterator->minmax_condition) {
		err = err_ok;
	}

	if (err == err_ok) {
		if (!iterator->minmax_condition) {
			get_field_list("ORDERBY", orderby_clause, orderby_fields);
		}

		iterator->orderby_condition = boolean_true;

		char *sort_pointer = strstr(orderby_fields, "ASC");

		if (NULL != sort_pointer) {
			iterator->orderby_asc = boolean_true;
		}

		ion_record_t sorted_records[3];

		sort(dictionary, sorted_records, iterator->orderby_asc, iterator->groupby_condition);

		if (iterator->minmax_condition) {
			iterator->sorted_records[1].key = NULL;
			iterator->sorted_records[2].key = NULL;

			/* Evaluate WHERE condition */
			if (iterator->where_condition) {
				int i = 0;

				while (i < 3) {
					if (0 == strncmp(sorted_records[i].value, "100", 3)) {
						iterator->sorted_records[0] = sorted_records[i];
						break;
					}

					i++;
				}
			}
			else {
				iterator->sorted_records[0] = sorted_records[0];
			}
		}
		else {
			iterator->sorted_records[0] = sorted_records[0];
			iterator->sorted_records[1] = sorted_records[1];
			iterator->sorted_records[2] = sorted_records[2];
		}
	}

	/* Evaluate SELECT here */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	if (0 != strncmp("*", select_fields, 1)) {
		/* If query is not SELECT * then it is SELECT fieldlist */
		iterator->select_fieldlist = boolean_true;
	}

	ion_dict_cursor_t *cursor = malloc(sizeof(ion_dict_cursor_t));

	dictionary_find(dictionary, &predicate, &cursor);

	/* Initialize iterator */
	iterator->record.key	= malloc((size_t) dictionary->instance->record.key_size);
	iterator->record.value	= malloc((size_t) dictionary->instance->record.value_size);
	iterator->cursor		= cursor;
	iterator->next			= next;
	iterator->destroy		= destroy;

	free(dictionary);

	return err_ok;
}

int
main(
	void
) {
	/* Cleanup just in case */
	table_cleanup();

	/* For SELECT * queries */
	table_setup();

	/* For SELECT fieldlist queries */
	fieldlist_table_setup();

	/* For ORDERBY fieldlist and GROUPBY queries */
	groupby_table_setup();

	/* Query */
	ion_query_iterator_t iterator;

	/* Set of basic operations for SELECT * */
/*	SQL_query(&iterator, "Select * FRoM test1.inq"); */
/*	SQL_query(&iterator, "Select * FRoM test1.inq where Key < 3"); */
/*	SQL_query(&iterator, "Select * FRoM test1.inq where Key < 3 orDerby key ASC"); */
/*	SQL_query(&iterator, "Select * FRoM test1.inq where Key < 3 orDerby key DESC"); */
/*	SQL_query(&iterator, "Select * FROM test1.inq orderby key ASC"); */
/*	SQL_query(&iterator, "Select * FROM test1.inq orderby key DESC"); */

	/* Set of basic operations for SELECT FIELDLIST */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq where Key < 3"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq where Key < 3 orderby key ASC"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq where Key < 3 orderby key DESC"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq orderby key ASC"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq orderby key DESC"); */

	/* Set of AGGREGATE operations */
/*	SQL_query(&iterator, "Select MAX(key) FRoM test1.inq"); */
/*	SQL_query(&iterator, "Select MAX(key), col1, col2 FRoM test3.inq where col1 = 100"); */
/*	SQL_query(&iterator, "Select MIN(key) FRoM test1.inq"); */
/*	SQL_query(&iterator, "Select MIN(key), col1, col2 FRoM test3.inq where col1 = 100"); */
/*	SQL_query(&iterator, "Select SUM(col2) FRoM test3.inq"); */
/*	SQL_query(&iterator, "Select SUM(col2) FRoM test3.inq where col1 = 100"); */
/*	SQL_query(&iterator, "Select AVG(col2) FRoM test3.inq"); */
/*	SQL_query(&iterator, "Select AVG(col2) FRoM test3.inq where col1 = 100"); */
	SQL_query(&iterator, "Select COUNT(*) FRoM test3.inq");
/*	SQL_query(&iterator, "Select COUNT(*) FRoM test3.inq where col1 = 100"); */

	/* Set of GROUPBY operations */
/*	SQL_query(&iterator, "Select key, col1, col2, FRoM test3.inq groupby col1"); */

	/* Iterate through results */
	iterator.record = next(&iterator, iterator.record);

	printf("\n");

	while (iterator.record.key != NULL) {
		if (!(iterator.sum_condition) && !(iterator.avg_condition) && !(iterator.count_condition)) {
			printf("Key: %i ", get_int(iterator.record.key));
		}

		if (iterator.groupby_condition || (iterator.minmax_condition && iterator.where_condition)) {
			printf("col1: %.*s ", 3, (char *) (iterator.record.value));
			printf("col2: %s\n", (char *) (iterator.record.value + 3));
		}
		else if (iterator.sum_condition || iterator.avg_condition) {
			printf("col2: %s\n", (char *) (iterator.record.value + 3));
		}
		else if (iterator.count_condition) {
			printf("COUNT: %s\n", (char *) iterator.record.value);
		}
		else if (iterator.minmax_condition && !iterator.where_condition) {
			printf("\n");
		}
		else if (iterator.select_fieldlist) {
			printf("Value: %s\n", (char *) (iterator.record.value));
		}
		else if (!iterator.minmax_condition) {
			printf("Value: %i\n", get_int(iterator.record.value));
		}

		iterator.record = next(&iterator, iterator.record);
	}

	return 0;
}
