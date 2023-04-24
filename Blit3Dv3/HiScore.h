#pragma once
#include <string>
#include <vector>

class HiScore 
{
public:
	std::string name;
	int score;
};

class ScoreTable
{
public:
	std::vector<HiScore> table;

	void Load();
	void Draw();
	void SortTable();
};