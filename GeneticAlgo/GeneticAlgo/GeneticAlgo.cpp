#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

struct Subject {
    std::string name;
    int hours;
};

struct Group 
{
    std::string name;
    int studentCount;
    std::vector<Subject> subjects;
};

struct Room 
{
    std::string name;
    int capacity;
};

struct Teacher 
{
    std::string name;
    int hours;
    std::vector<std::string> subjects;
};

struct ScheduleSlot 
{
    Group group;
    Subject subject;
    Teacher teacher;
    Room room;
    int timeSlot;
};

struct Schedule 
{
    std::vector<ScheduleSlot> slots;
    int fitness;
};

std::vector<Group> loadGroups(const std::string& fileName) {
    std::vector<Group> groups;
    std::ifstream file(fileName);
    std::string line;
    while (std::getline(file, line)) 
    {
        std::istringstream stream(line);
        Group group;
        std::getline(stream, group.name, ',');
        stream >> group.studentCount;
        groups.push_back(group);
    }
    return groups;
}

void loadSubjects(const std::string& fileName, std::vector<Group>& groups) {
    std::ifstream file(fileName);
    std::string line;
    while (std::getline(file, line)) 
    {
        std::istringstream stream(line);
        std::string groupName, subjectName;
        int hours;
        std::getline(stream, groupName, ',');
        std::getline(stream, subjectName, ',');
        stream >> hours;

        auto it = std::find_if(groups.begin(), groups.end(), [&groupName](const Group& g) {
            return g.name == groupName;
            });

        if (it != groups.end()) 
        {
            it->subjects.push_back({ subjectName, hours });
        }
    }
}

std::vector<Room> loadRooms(const std::string& fileName) {
    std::vector<Room> rooms;
    std::ifstream file(fileName);
    std::string line;
    while (std::getline(file, line)) 
    {
        std::istringstream stream(line);
        Room room;
        std::getline(stream, room.name, ',');
        stream >> room.capacity;
        rooms.push_back(room);
    }
    return rooms;
}

std::vector<Teacher> loadTeachers(const std::string& fileName) {
    std::vector<Teacher> teachers;
    std::ifstream file(fileName);
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream stream(line);
        Teacher teacher;
        std::getline(stream, teacher.name, ',');
        std::string hours;
        std::getline(stream, hours, ',');
        teacher.hours = std::stoi(hours);
        std::string subject;
        while (std::getline(stream, subject, ',')) 
        {
            teacher.subjects.push_back(subject);
        }
        teachers.push_back(teacher);
    }
    return teachers;
}

Schedule generateRandomSchedule(const std::vector<Group>& groups, const std::vector<Room>& rooms, const std::vector<Teacher>& teachers) {
    Schedule schedule;
    for (const auto& group : groups) 
    {
        for (const auto& subject : group.subjects) 
        {
            ScheduleSlot slot;
            slot.group = group;
            slot.subject = subject;
            slot.teacher = teachers[rand() % teachers.size()];
            slot.room = rooms[rand() % rooms.size()];
            slot.timeSlot = rand() % 20;
            schedule.slots.push_back(slot);
        }
    }
    return schedule;
}

int calculateFitness(const Schedule& schedule, const std::vector<Group>& groups, const std::vector<Room>& rooms, const std::vector<Teacher>& teachers) {
    int fitness = 0;
    for (const auto& slot : schedule.slots) 
    {
        for (const auto& other : schedule.slots) 
        {
            if (slot.timeSlot == other.timeSlot) 
            {
                if (slot.teacher.name == other.teacher.name && slot.room.name != other.room.name) fitness += 10;
                if (slot.group.name == other.group.name && slot.subject.name != other.subject.name) fitness += 10;
                if (slot.room.name == other.room.name && slot.group.name != other.group.name) fitness += 10;
                if (slot.teacher.name == other.teacher.name && std::find(slot.teacher.subjects.begin(), slot.teacher.subjects.end(), slot.subject.name) == slot.teacher.subjects.end()) fitness += 10;
            }
        }

        auto room = std::find_if(rooms.begin(), rooms.end(), [&slot](const Room& r) { return r.name == slot.room.name; });
        auto group = std::find_if(groups.begin(), groups.end(), [&slot](const Group& g) { return g.name == slot.group.name; });
        if (room != rooms.end() && group != groups.end() && group->studentCount > room->capacity) fitness += 5;
    }
    for (const auto& teacher : teachers)
    {
        int hoursCounter = 0;
        for (const auto& slot : schedule.slots)
        {
            if (teacher.name == slot.teacher.name)
                hoursCounter += 2;
        }
        if (hoursCounter > 10)
        {
            fitness += hoursCounter - 9;
        }
    }
    return fitness;
}

void selection(std::vector<Schedule>& population) 
{
    std::sort(population.begin(), population.end(), [](const Schedule& a, const Schedule& b) {
        return a.fitness < b.fitness;
        });
    population.resize(population.size() / 2);
}

void crossover(Schedule& parent1, Schedule& parent2, Schedule& offspring) 
{
    offspring.slots.clear();
    size_t crossoverPoint = parent1.slots.size() / 2;
    offspring.slots.insert(offspring.slots.end(), parent1.slots.begin(), parent1.slots.begin() + crossoverPoint);
    offspring.slots.insert(offspring.slots.end(), parent2.slots.begin() + crossoverPoint, parent2.slots.end());
}

void mutate(Schedule& schedule, const std::vector<Group>& groups, const std::vector<Room>& rooms, const std::vector<Teacher>& teachers) {
    for (auto& slot : schedule.slots) 
    {
        if (rand() % 100 < 25) {
            int mutationType = rand() % 5;

            switch (mutationType) {
            case 0: 
                slot.timeSlot = rand() % 20;
                break;
            case 1: 
                slot.teacher = teachers[rand() % teachers.size()];
                break;
            case 2: 
                slot.room = rooms[rand() % rooms.size()];
                break;
            case 3: 
                slot.group = groups[rand() % groups.size()];
                break;
            case 4: 
                std::vector<int> mask;
                for (int i = 0; i < 4; i++)
                {
                    mask.push_back(rand() % 2);
                }
                if (mask[0] == 1)
                    slot.timeSlot = rand() % 20;
                if (mask[1] == 1)
                    slot.teacher = teachers[rand() % teachers.size()];
                if (mask[2] == 1)
                    slot.room = rooms[rand() % rooms.size()];
                if (mask[3] == 1)
                    slot.group = groups[rand() % groups.size()];
                break;
            }
        }
    }
}

void rain(std::vector<Schedule>& population, const std::vector<Group>& groups, const std::vector<Room>& rooms, const std::vector<Teacher>& teachers)
{
    std::cout << "RAIN" << std::endl;
    for (int i = 1; i <= 15; ++i) 
    {
        population[population.size() - i] = generateRandomSchedule(groups, rooms, teachers);
        population[population.size() - i].fitness = calculateFitness(population[population.size() - i], groups, rooms, teachers);
    }
}

void saveScheduleToCSV(const Schedule& schedule, const std::string& fileName) {
    std::ofstream file(fileName);
    file << "Group,Subject,Teacher,Room,TimeSlot\n";
    for (const auto& slot : schedule.slots) 
    {
        file << slot.group.name << "," << slot.subject.name << "," << slot.teacher.name << "," << slot.room.name << "," << slot.timeSlot << "\n";
    }
    file.close();
}

bool checkMatches(std::vector<int> v)
{
    for (size_t i = 0; i < v.size() - 1; i++)
    {
        if (v[i] != v[i + 1])
            return false;
    }
    return true;
}

int main() {
    std::vector<Group> groups = loadGroups("E:\\groups.csv");
    loadSubjects("E:\\subjects.csv", groups);
    std::vector<Room> rooms = loadRooms("E:\\rooms.csv");
    std::vector<Teacher> teachers = loadTeachers("E:\\teachers.csv");
    std::vector<int> lastFitnesses;
    int lastFitnessesSize = 4;

    const int populationSize = 50;
    const int generations = 150;
    std::vector<Schedule> population;

    for (int i = 0; i < populationSize; ++i) 
    {
        population.push_back(generateRandomSchedule(groups, rooms, teachers));
        population[i].fitness = calculateFitness(population[i], groups, rooms, teachers);
    }

    for (int gen = 0; gen < generations; ++gen)
    {
        selection(population);
        std::vector<Schedule> offsprings;
        int index = 1;
        for (size_t i = 0; i < population.size(); i++) 
        {
            Schedule offspring;
            crossover(population[i % population.size()], population[(i + 1) % population.size()], offspring);
            mutate(offspring, groups, rooms, teachers);
            offspring.fitness = calculateFitness(offspring, groups, rooms, teachers);
            offsprings.push_back(offspring);
        }
        for (size_t i = 0; i < offsprings.size(); i++)
        {
            population.push_back(offsprings[i]);
        }

        std::sort(population.begin(), population.end(), [](const Schedule& a, const Schedule& b) {
            return a.fitness < b.fitness;
            });
        population.resize(populationSize);


        lastFitnesses.push_back(population[0].fitness);
        if (lastFitnesses.size() > lastFitnessesSize)
            lastFitnesses.erase(lastFitnesses.begin());
        if (lastFitnesses.size() >= lastFitnessesSize && checkMatches(lastFitnesses))
            rain(population, groups, rooms, teachers);

        std::cout << "Generation " << gen << " best fitness: " << population[0].fitness << std::endl;
    }

    saveScheduleToCSV(population[0], "E:\\final_schedule.csv");
    return 0;
}