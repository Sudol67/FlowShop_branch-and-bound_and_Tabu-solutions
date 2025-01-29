#include <string>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>


class MachineBlock {
	int start;
	int end;
	int taskNumber;
	int operationNumber;
	std::string blockType;
public:
	MachineBlock(int start, int end, int taskNumber, int operationNumber, std::string blockType) {
		this->start = start;
		this->end = end;
		this->taskNumber = taskNumber;
		this->operationNumber = operationNumber;
		this->blockType = blockType;
	}
	int get_start() {
		return start;
	}
	int get_end() {
		return end;
	}
	int get_taskNumber() {
		return taskNumber;
	}
	int get_operationNumber() {
		return operationNumber;
	}
	std::string get_blockType() {
		return blockType;
	}
};


class Task {
	int taskNumber;
	int operation1;
	int operation2;
public:
	Task(int taskNumber, int operation1, int operation2) {
		this->taskNumber = taskNumber;
		this->operation1 = operation1;
		this->operation2 = operation2;
	}
	int get_task_number() {
		return taskNumber;
	}
	int get_op1() {
		return operation1;
	}
	int get_op2() {
		return operation2;
	}
};

//Przeciazenie, dodawanie maintenance
void addBlock(std::vector <MachineBlock> &machineBlocks, const int start, const int end, const std::string type, const int taskNumber = 0,
              const int operationNumber = 0, const int maintenance = 0, const int taskLength = 0, const int maxTimeWork = 0);

void getUserInput(std::vector <Task> &tasks) {
    int numberOfTasks, operation1, operation2;

    std::cout << "Prosze podac liczbe zadan:" << std::endl;
    std::cin >> numberOfTasks;

    for (int taskNumber = 0; taskNumber < numberOfTasks; ++taskNumber) {
        std::cout << "Prosze podac dlugosc trwania operacji zadania " << (taskNumber + 1) << " (format: operacja1 o):" << std::endl;
        std::cin >> operation1 >> operation2;
        Task obj(taskNumber, operation1, operation2);
        tasks.push_back(obj);
    }
}

//Uzyskiwanie aktualnego Cmax z wybranej maszyny
int getCmax(std::vector <MachineBlock> &machineBlocks) {
	int machineBlocksSize = machineBlocks.size();
	if (machineBlocksSize)
		return machineBlocks[machineBlocksSize - 1].get_end();
	return 0;
}

//Uzyskiwanie ostatniego czasu niedostepnosci z wybranej maszyny
int lastMaintenance(std::vector <MachineBlock> &machineBlocks) {
	if (machineBlocks.size()) {
		int lastMaintenanceEnd = 0;
		for (MachineBlock machineBlock : machineBlocks) {
			if (machineBlock.get_blockType() == "maintenance")
				lastMaintenanceEnd = machineBlock.get_end();
		}
		return lastMaintenanceEnd;
	}
	return 0;
}
//-------------------------------------------------------------------------------------------------------------------------
//przewidywanie gdzie zakonczy sie zadanie na wybranej maszynie
int fitTaskOnMachine(std::vector <MachineBlock> &machine1Blocks, std::vector <MachineBlock> &machine2Blocks, const int maintenance, const int taskLength, const int maxTimeWork) {
    int start = getCmax(machine1Blocks);
    int lastMaintenanceEnd = lastMaintenance(machine1Blocks);
    if (start - lastMaintenanceEnd + taskLength > maxTimeWork)
		return start + taskLength + maintenance;
	return  start + taskLength;
}

//Dodawanie zadan do maszyn
void addBlock(std::vector <MachineBlock> &machineBlocks, int start, const int end, const std::string type, const int taskNumber, const int operationNumber, const int maintenance, const int taskLength, const int maxTimeWork) {
	int lastMaintenanceEnd = lastMaintenance(machineBlocks);
	MachineBlock block(start, end, taskNumber, operationNumber, type);
    machineBlocks.push_back(block);

	/*if (maxTimeWork != 0 && start - lastMaintenanceEnd + taskLength > maxTimeWork) {
		addBlock(machineBlocks, start+maintenance, start+maintenance+maintenance, "maintenance");
        start = getCmax(machineBlocks);
		MachineBlock block(start, start + taskLength, taskNumber, operationNumber, type);
		machineBlocks.push_back(block);
	}
	else {
		MachineBlock block(start, end, taskNumber, operationNumber, type);
		machineBlocks.push_back(block);
	}*/
}

void addTaskToMachines(std::vector <Task> &tasks, std::vector <MachineBlock> &machine1Blocks, std::vector <MachineBlock> &machine2Blocks, const int maintenance, const int maxTimeWork, std::vector <int> orderedTask) {
	for (int i = 0; i < orderedTask.size(); i++) { //wykonuje sie tyle razy ile w danym momencie musi uporzadkowac zadan

        for (int i = 0; i < orderedTask.size(); ++i) {
            std::cout << orderedTask[i] << " ";
        }
        std::cout << std::endl;
        //if (getCmax(machine1Blocks) - lastMaintenance(machine1Blocks) == maxTimeWork)
			//addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + maintenance, "maintenance");
		//if (getCmax(machine2Blocks) - lastMaintenance(machine2Blocks) == maxTimeWork)
			//addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + maintenance, "maintenance");


		int expectedMachine1End = fitTaskOnMachine(machine1Blocks, machine2Blocks, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork); //obliczanie miejsca zakończenia aktualnie rozpatrywanego zadania na 1 maszynie
        if(expectedMachine1End < getCmax(machine2Blocks)){
            expectedMachine1End = getCmax(machine2Blocks);
        }
        if(expectedMachine1End - lastMaintenance(machine1Blocks) > maxTimeWork){
            //Trzeba wstawic maintenance na M1
            if((getCmax(machine1Blocks) + maintenance + tasks[orderedTask[i]].get_op1() + tasks[orderedTask[i]].get_op2()) - lastMaintenance(machine2Blocks) > maxTimeWork){
                //Potrzeba wstawic maintenance na M1 i M2
                if(tasks[orderedTask[i]].get_op1() >= (getCmax(machine2Blocks) - getCmax(machine1Blocks))){
                    //Jesli dodawane na M1 zadanie jest wieksze lub rowne od istniejacego ostatniego na M2 to M1 decyduje o polozeniu
                    addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + maintenance, "maintenance");
                    if((getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1() - maintenance) - lastMaintenance(machine2Blocks) > maxTimeWork){
                        //Limitacja umiejscowienia maintenance na M2 maksymalnie skutecznie
                        addBlock(machine2Blocks, lastMaintenance(machine2Blocks) + maxTimeWork, lastMaintenance(machine2Blocks) + maxTimeWork + maintenance, "maintenance");
                    }
                    else{
                        addBlock(machine2Blocks, getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1()- maintenance, getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "maintenance");
                    }
                    addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                    addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);

                }
                else{
                    //Jesli dodawane na M1 zadanie jest mniejsze od istniejcego ostatniego na M2 to M2 decyduje o polozeniu
                    addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + maintenance, "maintenance");
                    if((getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1() - maintenance) - lastMaintenance(machine1Blocks) > maxTimeWork){
                        //Limitacja umiejscownienia maintennance na M1 maksymalnie skutecznie
                        addBlock(machine1Blocks, lastMaintenance(machine1Blocks) + maxTimeWork, lastMaintenance(machine1Blocks) + maxTimeWork + maintenance, "maintenance");
                    }
                    else{
                        addBlock(machine1Blocks, (getCmax(machine2Blocks) - maintenance) - tasks[orderedTask[i]].get_op1(), getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1(), "maintenance");
                    }
                    addBlock(machine1Blocks, getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1(), getCmax(machine2Blocks), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                    addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);
                }
            }
            else{
                //Potrzeba na M1 ale nie trzeba na M2
                if(getCmax(machine2Blocks) - getCmax(machine1Blocks) >= maintenance + tasks[orderedTask[i]].get_op1()){
                    addBlock(machine1Blocks, getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1() - maintenance, getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1(), "maintenance");
                    addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine2Blocks), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                    addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);
                }
                else{
                    addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + maintenance, "maintenance");
                    addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                    addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);
                }
            }
        }
        else{
            //Na pierwszy rzut oka nie trzeba dodawać maintenance na M1
            if((expectedMachine1End +  tasks[orderedTask[i]].get_op2()) - lastMaintenance(machine2Blocks) > maxTimeWork){
                //Ale trzeba dodac na M2
                if((expectedMachine1End + maintenance) - lastMaintenance(machine1Blocks) > maxTimeWork){
                    //Jesli jednak po dodaniu do M2 potrzebujemy na M1
                    if(tasks[orderedTask[i]].get_op1() >= (getCmax(machine2Blocks) - getCmax(machine1Blocks))){
                        //Jesli dodawane na M1 zadanie jest wieksze od istniejacego ostatniego na M2 to M1 decyduje o polozeniu
                        addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + maintenance, "maintenance");
                        if((getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1() - maintenance) - lastMaintenance(machine2Blocks) >= maxTimeWork){
                            //Limitacja umiejscowienia maintenance na M2 maksymalnie skutecznie
                            addBlock(machine2Blocks, lastMaintenance(machine2Blocks) + maxTimeWork, lastMaintenance(machine2Blocks) + maxTimeWork + maintenance, "maintenance");
                        }
                        else{
                            addBlock(machine2Blocks, getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1() - maintenance, getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "maintenance");
                        }
                        addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                        addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);

                    }
                    else{
                        //Jesli dodawane na M1 zadanie jest mniejsze od istniejcego ostatniego na M2 to M2 decyduje o polozeniu
                        addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + maintenance, "maintenance");
                        if((getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1() - maintenance) - lastMaintenance(machine1Blocks) > maxTimeWork){
                            //Limitacja umiejscownienia maintennance na M1 maksymalnie skutecznie
                            addBlock(machine1Blocks, lastMaintenance(machine1Blocks) + maxTimeWork, lastMaintenance(machine1Blocks) + maxTimeWork + maintenance, "maintenance");
                        }
                        else{
                            addBlock(machine1Blocks, (getCmax(machine2Blocks) - maintenance) - tasks[orderedTask[i]].get_op1(), getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1(), "maintenance");
                        }
                        addBlock(machine1Blocks, getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1(), getCmax(machine2Blocks), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                        addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);
                    }
                }
                else{
                    //Nie potrzeba na M1 ale potrzeba na M2
                    if(tasks[orderedTask[i]].get_op1() > (getCmax(machine2Blocks) - getCmax(machine1Blocks)) + maintenance){
                        if(((getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1()) - maintenance) - lastMaintenance(machine2Blocks) > maxTimeWork){
                            //Limitacja umiejscowienia maintenance na M2 maksymalnie skutecznie
                            addBlock(machine2Blocks, lastMaintenance(machine2Blocks) + maxTimeWork, lastMaintenance(machine2Blocks) + maxTimeWork + maintenance, "maintenance");
                        }
                        else{
                            addBlock(machine2Blocks, (getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1()) - maintenance, getCmax(machine1Blocks), "maintenance");
                        }
                        addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                        addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);

                    }
                    else{
                        addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + maintenance, "maintenance");
                        addBlock(machine1Blocks, getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1(), getCmax(machine2Blocks), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                        addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);
                    }
                }
            }
            else{
                //Nie potrzeba ani na M1 ani na M2
                if(tasks[orderedTask[i]].get_op1() < (getCmax(machine2Blocks) - getCmax(machine1Blocks))){
                    addBlock(machine1Blocks, getCmax(machine2Blocks) - tasks[orderedTask[i]].get_op1(), getCmax(machine2Blocks), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                    addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);
                }
                else{
                    addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork);
                    addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//generowanie losowego rozmieszczenia zadan
std::vector <int> randomTasksOrder(const std::vector <Task> &tasks) {
    std::random_device rd;   // Urzadzenie do generowania liczb losowych
    std::mt19937 g(rd());
	std::vector <int> randomOrder;
	for (int i = 0; i < tasks.size(); i++)
		randomOrder.push_back(i);
	std::shuffle(randomOrder.begin(), randomOrder.end(), g);
	return randomOrder;
}

//https://stackoverflow.com/questions/33044735/c-generating-random-numbers-inside-loop/33045918
//liczenie czasu dzialania
std::default_random_engine dre(std::chrono::steady_clock::now().time_since_epoch().count());
int random(int lim) {
	std::uniform_int_distribution<int> uid{ 0, lim };
	return uid(dre);
}

//wybiera losowo task i zmienia jego pozycje na nowa, losowa
std::vector <int> newTasksOrder(std::vector <int> &orderedTasks) { //przekazywane jest aktualne rozwiazanie
	int selectedTaskPosition = random(orderedTasks.size() - 1); //wybranie zadania do zmiany pozycji
	int selectedTask = orderedTasks[selectedTaskPosition]; //przypisanie wybranego zadania
	orderedTasks.erase(orderedTasks.begin() + selectedTaskPosition); //usuniecie zadania z pierwotnej pozycji
	int newSelectedTaskPosition = random(orderedTasks.size() - 1); //losowe wybranie nowej pozycji
	orderedTasks.insert(orderedTasks.begin() + newSelectedTaskPosition, selectedTask); //wstawienie zadania na nowa pozycje
	return orderedTasks; //zwrocenie zmienionego rozwiazania
}
//wyswietlanie
void printResults(std::vector <std::vector <MachineBlock>> bestMachinesBlocks) {
    std::cout << "MACHINE 1" << std::endl;
    for (int i = 0; i < bestMachinesBlocks[0].size(); i++) {
        std::cout << bestMachinesBlocks[0][i].get_blockType();
        if (bestMachinesBlocks[0][i].get_taskNumber() != 0)
            std::cout << "_" << bestMachinesBlocks[0][i].get_taskNumber();
        std::cout << " |" << bestMachinesBlocks[0][i].get_start() << ":";
        std::cout << bestMachinesBlocks[0][i].get_end() << "|";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "MACHINE 2" << std::endl;
    for (int i = 0; i < bestMachinesBlocks[1].size(); i++) {
        std::cout << bestMachinesBlocks[1][i].get_blockType();
        if (bestMachinesBlocks[1][i].get_taskNumber() != 0)
            std::cout << "_" << bestMachinesBlocks[1][i].get_taskNumber();
        std::cout << " |" << bestMachinesBlocks[1][i].get_start() << ":";
        std::cout << bestMachinesBlocks[1][i].get_end() << "|";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Cmax = " << getCmax(bestMachinesBlocks[1]) << std::endl;
}

void tabuSearch(std::vector <Task> &tasks, std::vector <MachineBlock> &machine1Blocks, std::vector <MachineBlock> &machine2Blocks, const int maintenance, const int maxTimeWork, int neighborhood, int newInitialSolutionCount) {
	auto start = std::chrono::high_resolution_clock::now();
	std::vector < std::vector<int> > tabuList; //lista tabu
	int tabuListMaxSize = ceil(0.2 * tasks.size()); //ustawianie wielkości listy tabu
	std::vector <std::vector <MachineBlock>> bestCandidate; //deklaracja zmiennej bestCandidate jako wektor wektorów obiektów MachineBlock. BestCandidate to najlepsze dotychczas rozwiązanie
	while (newInitialSolutionCount != 0) { //pętla która będzie sie wykonywała tyle razy ile ustawi uzytkownik
		machine1Blocks.clear(); //Clear maszyny 1
		machine2Blocks.clear(); //Clear maszyny 2
		std::vector <int> initialSolution = randomTasksOrder(tasks); //tworzenie rozwiazania poczatkowego poprzez wylosowanie ustawienia zadan
		addTaskToMachines(tasks, machine1Blocks, machine2Blocks, maintenance, maxTimeWork, initialSolution); //ustawianie zadan na maszynie z uwzglednieniem maintenance
		if (bestCandidate.size() == 0) { //jesli nowe rozwiazanie poczatkowe jest pierwszym to dodajemy jako najlepsze:
			bestCandidate.push_back(machine1Blocks); //dodajemy ulozenie na maszynie 1
			bestCandidate.push_back(machine2Blocks); //dodajemy ulozenie na maszynie 2
			machine1Blocks.clear(); //czyszczenie
			machine2Blocks.clear();
		}
		else if (getCmax(machine2Blocks) < getCmax(bestCandidate[1])) { //jesli nowe rozwiazanie poczatkowe jest lepsze od dotychczasowego to zmieniamy
			bestCandidate.clear();
			bestCandidate.push_back(machine1Blocks);
			bestCandidate.push_back(machine2Blocks);
			machine1Blocks.clear();
			machine2Blocks.clear();
		}
		int improvementCount = ceil(0.5 * tasks.size()); //ustawiamy zmienna iteracji bez poprawy
		int neighborhoodIndex = neighborhood; //sasiedztwo
		while (neighborhoodIndex > 0) { //dopoki zmienna iteracji bez poprawy jest wieksza niz 0 to szukamy kandydata
			std::vector <int> newCandidate = newTasksOrder(initialSolution); //zadania sa w sposob losowy przetasowywane
			if (find(tabuList.begin(), tabuList.end(), newCandidate) == tabuList.end()) { //sprawdzenie czy wygenerowany kandydat jest na liscie tabu. jesli nie, jest dalej rozpatrzany. jesli tak to wybierany jest nowy
				if (tabuList.size() == tabuListMaxSize) //usuwanie elementu z listy tabu jesli przekroczy limit
					tabuList.erase(tabuList.begin());
				tabuList.push_back(newCandidate); //dodanie nowego kandydata do listy tabu
				machine1Blocks.clear(); //wyczyszczenie tymczasowej pamieci maszyn zeby dodac nowego kandydata
				machine2Blocks.clear();//wyczyszczenie tymczasowej pamieci maszyn zeby dodac nowego kandydata
				addTaskToMachines(tasks, machine1Blocks, machine2Blocks, maintenance, maxTimeWork, newCandidate); //dodanie zadan i maintenance zgodnie z zasadami
				if (getCmax(machine2Blocks) < getCmax(bestCandidate[1])) { //Ewaluacja czy aktualne rozwiązanie jest lepsze od najlepszzego dotychczas jeśli tak to:
					bestCandidate.clear(); //czyszczone dotychczas najlepsze rozwiazanie
					bestCandidate.push_back(machine1Blocks); //dodanie najlepszego rozwiazania do maszyny 1
					bestCandidate.push_back(machine2Blocks); //dodanie najlepszego rozwiazania do maszyny 2
					machine1Blocks.clear(); //wyczyszczenie aktualnego kandydata rozwiazania maszyny 1
					machine2Blocks.clear(); //wyczyszczenie aktualnego kandydata rozwiazania maszyny 1
					initialSolution = newCandidate; //aktualne rozwiazanie = wlasnie uzyskane najlepsze rozwiazanie
				}
				else if (getCmax(machine2Blocks) == getCmax(bestCandidate[1])) { //Jesli nie jest lepsze to:
					improvementCount -= 1; //iteracja bez poprawy
					if (improvementCount == 0) //jesli iteracje spadną do limitu to:
						neighborhoodIndex = 0; //index ustawiany na 0 i koniec dzialania programu
				}
				neighborhoodIndex -= 1;
			}
		}
		newInitialSolutionCount -= 1;
	}
	auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	printResults(bestCandidate);
    std::cout << "Czas wykonania zadania: " << duration.count() << " milisekund" << std::endl;
}

int main() {
	int maintenance;
	int maxTimeWork;
	int neighborhood;
	int newInitialSolutionCount;

	std::cout << "Prosze podac dlugosc okresu niedostepnosci (int):" << std::endl;
	std::cin >> maintenance;
	std::cout << "Prosze podac maksymalny czas bez okresu niedostepnosci (int):" << std::endl;
	std::cin >> maxTimeWork;
	std::cout << "Prosze podac wielkosc sasiedztwa (int):" << std::endl;
	std::cin >> neighborhood;
	std::cout << "Prosze podac ile razy algorytm ma szukac nowego rozwiazania poczatkowego:" << std::endl;
	std::cin >> newInitialSolutionCount;

	std::vector <Task> tasks;
	std::vector <MachineBlock> machine1Blocks;
	std::vector <MachineBlock> machine2Blocks;
	getUserInput(tasks);
	std::cout << "Program rozpoczyna dzialanie..." << std::endl;

	tabuSearch(tasks, machine1Blocks, machine2Blocks, maintenance, maxTimeWork, neighborhood, newInitialSolutionCount);
}
