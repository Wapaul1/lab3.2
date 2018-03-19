// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 32 kilobytes available
// for the CBP-2 contest; it is just an example.
#include <algorithm>

class my_update : public branch_update {
public:
	unsigned int index;
        int output;
};

class my_predictor : public branch_predictor {
public:
#define HISTORY_LENGTH	59
#define TABLE_BITS	9
#define threshold       127
	my_update u;
	branch_info bi;
	unsigned long long int history;
	char perceps[1 << TABLE_BITS][HISTORY_LENGTH + 1];

	my_predictor (void) : history(0) { 
		memset (perceps, 0, sizeof (perceps));
	}

	branch_update *predict (branch_info & b) {
		bi = b;
		if (b.br_flags & BR_CONDITIONAL) {
			u.index = (history & ((1<<TABLE_BITS) - 1)) ^ (b.address & ((1<<TABLE_BITS)-1));
                        int output = perceps[u.index][0];
                        for (int i = 0; i < HISTORY_LENGTH; i++) {
                               if ((history & ( 1 << i )) >> i) {
                                 output += perceps[u.index][i+1];
                               } else {
                                 output -= perceps[u.index][i+1];
                               }
                        }
			u.direction_prediction (output >= 0);
                        u.output = output;
		} else {
			u.direction_prediction (true);
		}
		u.target_prediction (0);
		return &u;
	}

	void update (branch_update *u, bool taken, unsigned int target) {
		if (bi.br_flags & BR_CONDITIONAL) {
                        my_update* ut = (my_update*) u;
                        int num = 2 * taken - 1;
			if ((taken != (ut->output >= 0)) | (abs(ut->output) <= threshold)) {
                          perceps[ut->index][0] = std::max(std::min(perceps[ut->index][0] + num, 127), -128);
                          for (int i = 0; i < HISTORY_LENGTH; i++) {
                               if ((history & ( 1 << i )) >> i) {
                                 perceps[ut->index][i+1] = std::max(std::min(perceps[ut->index][i+1] + num, 127), -128);
                               } else {
                                 perceps[ut->index][i+1] = std::max(std::min(perceps[ut->index][i+1] - num, 127), -128);
                               }
                          }
			}
			history <<= 1;
			history |= taken;
			history &= (1<<HISTORY_LENGTH)-1;
		}
	}
};
