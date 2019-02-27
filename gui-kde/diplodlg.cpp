extern "C" {
#include "diplodlg_g.h"
}
#include "logging.h"

void handle_diplomacy_init_meeting(int counterpart, int initiated_from) {
  qCDebug(FC) << "handle_diplomacy_init_meeting";
}

void handle_diplomacy_cancel_meeting(int counterpart, int initiated_from) {
  qCDebug(FC) << "handle_diplomacy_cancel_meeting";
}

void handle_diplomacy_create_clause(int counterpart, int giver, enum clause_type type, int value) {
  qCDebug(FC) << "handle_diplomacy_create_clause";
}

void handle_diplomacy_remove_clause(int counterpart, int giver, enum clause_type type, int value) {
  qCDebug(FC) << "handle_diplomacy_remove_clause";
}

void handle_diplomacy_accept_treaty(int counterpart, bool I_accepted, bool other_accepted) {
  qCDebug(FC) << "handle_diplomacy_accept_treaty";
}

void close_all_diplomacy_dialogs() {
  qCDebug(FC) << "close_all_diplomacy_dialogs";
}

