/*
 * proj_consts.cpp
 *
 *  Created on: 2013-9-28
 *      Author: root
 */

#ifndef PROJ_CONSTS_CPP_
#define PROJ_CONSTS_CPP_

#include "proj_consts.hpp"

string CYEAR = "2015";

namespace proj_consts
{
	class TUnitController
	{
		public:
		TUnitController();
		~TUnitController();
	};

	TUnitController::TUnitController()
	{
		// Initialization
		CYEAR = FormatDateTime("yyyy", Now());
	}

	TUnitController::~TUnitController()
	{
		// Finalization
	}

	TUnitController ThisUnit;
}

#endif /* PROJ_CONSTS_CPP_ */
