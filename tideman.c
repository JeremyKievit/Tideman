#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void record_preference(int index, int ranks[]);
void add_pairs(void);
void sort_pairs(void);
int divide(pair sorted[], int index, int count);
void sort(pair sorted[], int sorted_index, int index, int count);
void lock_pairs(void);
void print_winner(void);
int chase_source(int candidate_index);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(name, candidates[i]) == 0)
        {
            ranks[rank] = i;
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    record_preference(0, ranks);
    return;
}

void record_preference(int wnr_indx, int ranks[])
{
    // candidate at ranks[index] is preferred over the remaining candidates in the set
    int winner = ranks[wnr_indx];
    for (int lsr_indx = wnr_indx + 1; lsr_indx < candidate_count; lsr_indx++)
    {
        // adds a vote to the preferences array at [i][j], where [i] is preferred over [j]
        int loser = ranks[lsr_indx];
        preferences[winner][loser] += 1;
    }

    // recursive call sets the next candidate (index + 1) preferred over the remaining candidates
    if (wnr_indx + 2 < candidate_count)
    {
        record_preference(wnr_indx + 1, ranks);
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    int pairs_index = 0;

    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            if (preferences[i][j] > preferences[j][i])
            {
                pairs[pairs_index].winner = i;
                pairs[pairs_index].loser = j;
                pairs_index++;
            }
        }
    }

    pair_count = pairs_index;
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    //sort divides the parent set, then merges each recursive set into a sorted array
    pair sorted_pairs[pair_count];
    divide(sorted_pairs, 0, pair_count);

    printf("\n\nunsorted pairs:\n");
    for (int i = 0; i < pair_count; i++)
    {
        printf("candidate %d -> ", pairs[i].winner);
        printf("%d\n", pairs[i].loser);
    }

    printf("\nsorted pairs:\n");
    for (int i = 0; i < pair_count; i++)
    {
        pairs[i] = sorted_pairs[i];

        printf("candidate %d -> ", pairs[i].winner);
        printf("%d\n", pairs[i].loser);
    }
    printf("\n");
    return;
}

int divide(pair sorted[], int start, int end)
{
    int sorted1 = 1;
    int sorted2 = 1;

    if ((end - start) > 1) {
        // subdivides the indices if the number of items between them is greater than 1
        sorted1 = divide(sorted, start, end/2);
        sorted2 = divide(sorted, (start + end/2), end);

    }
    // merges each set of sorted pairs
    sort(sorted, (sorted1 + sorted2), start, end);

    //returns the count of the merged arrays
    return start + end;
}

void sort(pair sorted[], int sorted_index, int start, int end)
{

    // the index of a set (set1_index, set2_index) points at the next item to be sorted
    int midpoint = (end - start)/2;
    int set2_index = (end - start)/2;

    for (int set1_index = start; set1_index <= midpoint; set1_index++)
    {
        if (set2_index == end)
        {
            //set2 is fully merged, so the remaining items from set1 are appended to the sorted set
            sorted[sorted_index] = pairs[set1_index];
            sorted_index++;

            continue;
        }

        int winner1 = pairs[set1_index].winner;
        for (int i = set2_index; i <= end; i++)
        {
            if (set1_index == midpoint && midpoint != start)
            {
                //set1 is fully merged, so the remaining items from set2 are appended to the sorted set
                sorted[sorted_index] = pairs[i];
                sorted_index++;
                continue;
            }

            int winner2 = pairs[set2_index].winner;
            if (preferences[winner1][winner2] == 0 || preferences[winner2][winner1] == 0)
            {
                continue;
            }

            if (preferences[winner1][winner2] > preferences[winner2][winner1])
            {
                // winner1 is preferred over winner2, so winner1 is added to the sorted array, and execution "breaks" to set1_index_+1
                sorted[sorted_index] = pairs[set1_index];
                sorted_index++;

                break;
            }
            else if (preferences[winner1][winner2] <= preferences[winner2][winner1])
            {
                // winner2 is preferred over winner1, so winner2 is added to the sorted array, and execution continues to i+1
                sorted[sorted_index] = pairs[set2_index];
                set2_index++;
                sorted_index++;
            }
        }
    }
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    int winner;
    int loser;
    int cycle;
    int source;
    bool source_updated = false;

    for (int pair_index = 0; pair_index < pair_count; pair_index++)
    {
        winner = pairs[pair_index].winner;
        loser = pairs[pair_index].loser;
        cycle = false;

        if (source_updated == false)
        {
            source = winner;
            source_updated = true;
        }

        for (int i = 0; i < candidate_count; i++)
        {
            for (int j = 0; j < candidate_count; j++)
            {
                // there can be only one edge pointing to a candidate (each candidate can lose only once)
                if (locked[i][j] == true && j == loser)
                {
                    cycle = true;
                }
                // the source of the graph can have and edge pointing to it only if the winner is the new source
                // if any loser -> source, cycle = true
                else if (locked[i][j] == true && j == winner && loser == source)
                {
                    cycle = true;
                }
            }
        }
        if (!cycle)
        {
            locked[winner][loser] = true;
            if (loser == source)
            {
                source = winner;
            }
        }

    }
    return;
}

// Print the winner of the election
void print_winner(void)
{
    int source = chase_source(0);
    printf("%s\n", candidates[source]);
    return;
}

int chase_source(candidate_index)
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[i][candidate_index] == true)
        {
            return chase_source(i);
        }
    }
    return candidate_index;
}
