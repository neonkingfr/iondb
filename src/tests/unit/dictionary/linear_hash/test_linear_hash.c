/******************************************************************************/
/**
@file
@author		Spencer MacBeth
@brief		Implementation unit tests for the linear hash.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#include "test_linear_hash.h"

void
test_linear_hash_create_destroy(
	planck_unit_test_t *tc
) {
	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true);
}

planck_unit_suite_t *
linear_hash_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_create_destroy);

	return suite;
}

void
runalltests_linear_hash(
) {
	planck_unit_suite_t *suite = linear_hash_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
