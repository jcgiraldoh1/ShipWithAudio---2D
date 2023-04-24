#include "HiScore.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include "Blit3D.h"
#include<algorithm>


extern AngelcodeFont* scorefont;




void ScoreTable::Load()
{
	std::ifstream scoreFile("highscore.txt");

	if (!scoreFile)
	{
		//couldnot open the file!
		assert(false);
		return;
	}

	for (int i = 0; i < 5; ++i)
	{
		//make a score HiScore
		HiScore H;

		//read the name
		scoreFile >> H.name;
		//read the score
		scoreFile >> H.score;
		//store the hiscore on the table
		table.push_back(H);
	}
}


void ScoreTable::Draw()
{
	for (int i = 0; i < table.size(); ++i)
	{
		scorefont->BlitText(680, 700 - i * 70, table[i].name);
		scorefont->BlitText(980, 700 - i * 70, std::to_string(table[i].score));
	}
}


//comparison function for soorting our hiscore
bool HiScoreSort(HiScore A, HiScore B)
{
	return A.score > B.score;
}

void ScoreTable::SortTable()
{
	//sort our hi score table
	std::sort(table.begin(), table.end(), HiScoreSort);

}