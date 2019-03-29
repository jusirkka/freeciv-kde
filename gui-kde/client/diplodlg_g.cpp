extern "C" {
#include "diplodlg_g.h"
}
#include "logging.h"
#include "application.h"
#include "diptreaty.h"

void handle_diplomacy_init_meeting(int counterpart, int /*initiated_from*/) {
  qCDebug(FC) << "handle_diplomacy_init_meeting";
  KV::Application::InitMeeting(counterpart);
}

void handle_diplomacy_cancel_meeting(int counterpart, int /*initiated_from*/) {
  qCDebug(FC) << "handle_diplomacy_cancel_meeting";
  KV::Application::CancelMeeting(counterpart);
}

void handle_diplomacy_create_clause(int counterpart, int giver, enum clause_type type, int value) {
  qCDebug(FC) << "handle_diplomacy_create_clause";
  Clause d;
  d.from = player_by_number(giver);
  d.type = type;
  d.value = value;
  KV::Application::CreateClause(counterpart, d);
}

void handle_diplomacy_remove_clause(int counterpart, int giver, enum clause_type type, int value) {
  qCDebug(FC) << "handle_diplomacy_remove_clause";
  Clause d;
  d.from = player_by_number(giver);
  d.type = type;
  d.value = value;
  KV::Application::RemoveClause(counterpart, d);
}

void handle_diplomacy_accept_treaty(int counterpart, bool /*I_accepted*/, bool other_accepted) {
  qCDebug(FC) << "handle_diplomacy_accept_treaty";
  KV::Application::AcceptTreaty(counterpart, other_accepted);
}

void close_all_diplomacy_dialogs() {
  qCDebug(FC) << "close_all_diplomacy_dialogs";
  KV::Application::CloseAllTreatyDialogs();
}

