/*********************************** * Filename: game.cpp * Last Modified: 2020/04/02
 * **********************************/


#include "game.h"
#include <algorithm> //std::copy for array copying, std::max_element
#include "misc.h"

using namespace std;

Game::Game() {

}

Game::~Game() {

}

int Game::AddPlayer(int seat, int stack_size, int type) {
	if (players[seat]->GetStatus() != STATUS_NO_PLAYER) {
		cerr << "Player already sitting at seat #" << seat << endl;
		cerr << "AddPlayer() failure" << endl;
		return -1;
    }	
	players[seat]->SetType(type);
    players[seat]->SetStatus(1); //#define STATUS_IN_GAME		1
	players[seat]->AddToStack(stack_size);
    players[seat]->SetID(seat);
	
    //Update game_state_
    game_state_.num_player++;
    game_state_.stack_size[seat] = stack_size;
    game_state_.starting_stack_size[seat] = stack_size;
    game_state_.bankroll[seat] = 10000 * 1000; //1000 buyins
    game_state_.player_status[seat] = 1; //1=in-game
	return 0;
}


int Game::AddPlayer(int seat, int stack_size, Player* player) {
    players[seat] = player;
    game_state_.num_player++;
    game_state_.stack_size[seat] = stack_size;
    game_state_.starting_stack_size[seat] = stack_size;
    game_state_.bankroll[seat] = 10000 * 1000; //1000 buyins
    game_state_.player_status[seat] = 1; //1=in-game
    players[seat]->SetID(seat);
}

void Game::RemovePlayer(int seat) {
	players[seat]->SetStatus(STATUS_NO_PLAYER);	
}

void Game::ShuffleAndDeal() {

	vector<Card> dealt_cards;
    dealt_cards.reserve(2);
    deck.Regenerate();
	deck.Shuffle();

//    #ifdef DEBUG
//        std::cout << "[Debug] Starting a hand" << std::endl;
//        std::cout << "[Debug] Deck is:";
//        deck.print();
//    #endif
	for (int i = 0; i < NUM_OF_PLAYERS; i++) {
        dealt_cards.clear();
		dealt_cards.push_back(deck.Deal());
		dealt_cards.push_back(deck.Deal());
		players[i]->SetHoleCards(dealt_cards[0]);
        players[i]->SetHoleCards(dealt_cards[1]);

        #ifdef DEBUG
        std::cout << "[Debug] Hole card of Player Seat" << i << ":";
        std::cout << dealt_cards[0] << dealt_cards[1] << std::endl;
        #endif
	}
}

void Game::PostBlinds() {
    ComputeBlindPos();
    std::cout << "[INFO] Player " << game_state_.sb_pos << " posts SB:" << game_state_.sb_amount << std::endl;
    std::cout << "[INFO] Player " << game_state_.bb_pos << " posts BB:" << game_state_.bb_amount << std::endl;
//No longer needs    players[game_state_.sb_pos]->Bet(game_state_.sb_amount);
//No longer needs    players[game_state_.bb_pos]->Bet(game_state_.bb_amount);
    
    //update game state
    game_state_.stack_size[game_state_.sb_pos] -= game_state_.sb_amount;
    game_state_.stack_size[game_state_.bb_pos] -= game_state_.bb_amount;
    game_state_.bet_ring[game_state_.sb_pos] = game_state_.sb_amount;
    game_state_.bet_ring[game_state_.bb_pos] = game_state_.bb_amount;
    game_state_.next_player_to_act = FindNextPlayer(game_state_.bb_pos);
    game_state_.aggressor = FindNextPlayer(game_state_.bb_pos); //last to act is BB
    game_state_.raise_amount = game_state_.bb_amount;
}

ActionWithID Game::AskPlayerToAct(LegalActions legal_actions) {

    #ifdef DEBUG
    std::cout << "[DEBUG] Asking player " << game_state_.next_player_to_act << " to act." \
                << "(call:" << legal_actions.LegalCall.amount << "," \
                << "min-raise:" << legal_actions.LegalMinRaise.amount << ")" << std::endl;
    #endif
    ActionWithID player_action_with_id;
    player_action_with_id.ID = game_state_.next_player_to_act;
    player_action_with_id.player_action = players[game_state_.next_player_to_act]->Act(game_state_);

	std::cout << "[INFO] Player " << player_action_with_id.ID << " " \
                    << player_action_with_id.player_action << std::endl;
    return player_action_with_id;
}

void Game::PrintGameState() {
    game_state_.print();
}

void Game::ComputeBlindPos() {
    if (game_state_.num_player < 2)
        std:cerr << "Player number is incorrect:" << game_state_.num_player << std::endl;
    

    game_state_.sb_pos = FindNextPlayer(game_state_.btn_pos);
    game_state_.bb_pos = FindNextPlayer(game_state_.sb_pos);

    if (game_state_.num_player == 2) {
        std::swap(game_state_.sb_pos, game_state_.bb_pos);
    }
}

int Game::FindNextPlayer(int i) {
    i++;
    while ( game_state_.player_status[i%9] != 1 || IsPlayerAllIn(i) ) {
        i++;
    }
    return i%9;
}

bool Game::IsPlayerAllIn(int i) {
    return  ( game_state_.stack_size[i] == 0 ) && ( game_state_.player_status[i] == 1 );
}

void Game::MoveBtn(){
    game_state_.btn_pos = FindNextPlayer(game_state_.btn_pos);
}


bool Game::IsPotUncontested() {
    if (game_state_.num_player_in_hand >= 2) {
        return 0;
    }
    else {
        return 1;
    }
}


bool Game::HasReachShowdown() {
    if (game_state_.current_street == 4)
        return 1;
    else
        return 0;
}

vector<int> Game::GetWinner(){
    vector<int> winner;

    if(game_state_.num_player_in_hand == 1) {
        for (int i = 0 ; i < 9 ; i++){
            if (game_state_.player_status[i] == 1) { // 1: in game, 2: folded
                winner.push_back(i);
            }
        }
    }
    else{
        vector<PokerHand> poker_hands;
        vector<int> showdown_player_index; 
        for (int i = 0 ; i < 9 ; i++) {
            if(game_state_.player_status[i] == 1) { //1: in game
                PokerHand temp;
                temp.add( players[i]->GetHoleCards().at(0) );
                temp.add( players[i]->GetHoleCards().at(1) );

                std::cout << "Player " << i << " shows " << players[i]->GetHoleCards().at(0) << players[i]->GetHoleCards().at(1) << std::endl;
                temp.add( game_state_.community_cards[0]);
                temp.add( game_state_.community_cards[1]);
                temp.add( game_state_.community_cards[2]);
                temp.add( game_state_.community_cards[3]);
                temp.add( game_state_.community_cards[4]);
                poker_hands.push_back(temp);
                showdown_player_index.push_back(i);
            }
        }

        for (int i = 0 ; i < poker_hands.size() ; i++ ) {
            if (winner.size() == 0) {
                winner.push_back(showdown_player_index[i]);
            }
            else if (poker_hands[i] > poker_hands[winner[0]] ) {
                winner.clear();
                winner.push_back(showdown_player_index[i]);
            } 
            else if (poker_hands[i] == poker_hands[winner[0]] ) {
                winner.push_back(showdown_player_index[i]);
            }
        }

    }
    std::cout<< "[INFO] Winner(s): " << winner << std::endl;
    return winner;
}


void Game::PayWinner(vector<int> winners){
    
    for(auto const& i: winners) {
        std::cout << "[INFO] Player " << i << " wins " << game_state_.pot_size/winners.size() << std::endl ;
        game_state_.stack_size[i] += game_state_.pot_size/winners.size();
    }

}

bool Game::IsEndOfStreet() {
    return game_state_.next_player_to_act == game_state_.aggressor ;
}

void Game::CollectMoneyFromBetRing() {

    for (int i = 0 ; i < 9 ; i++ ) {
        if (game_state_.bet_ring[i] == 0)
            continue;
        game_state_.pot_size += game_state_.bet_ring[i];
        game_state_.bet_ring[i] = 0;
    }

    #ifdef DEBUG
    std::cout << "[INFO] Dealer collects money from bet ring and put into pot" << std::endl ;
    #endif

    std::cout << "[INFO] Pot size is: " << game_state_.pot_size << std::endl;
}


void Game::SetupNextStreet() {

    game_state_.current_street += 1;
    
    int cards_to_deal = 0 ;

    switch (game_state_.current_street) {
        case 1:
            cards_to_deal = 3;
            break;

        case 2:
        case 3:
            cards_to_deal = 1;
            break;
        case 4:
            cards_to_deal = 0;
            break;
        default:
            std::cerr << "[Error] game_state_.current_street is out of bound: " << game_state_.current_street << std::endl;
    }

    #ifdef DEBUG
        std::cout << "[DEBUG] Dealer deals next street: " << game_state_.current_street << std::endl;
    #endif

    for (int i = 0 ; i < cards_to_deal ; i++ ) {
        game_state_.community_cards.push_back(deck.Deal());
    }

    std::cout << "[INFO] ***" << static_cast<StreetName> (game_state_.current_street) <<"*** ";
    for (auto const& card : game_state_.community_cards)
        std::cout << card << ' ';
    std::cout << std::endl;


    if (game_state_.num_player == 2) 
        game_state_.next_player_to_act = game_state_.bb_pos;
    else
        game_state_.next_player_to_act = FindNextPlayer(game_state_.bb_pos);
    game_state_.aggressor = FindNextPlayer(game_state_.btn_pos); 
    game_state_.raise_amount = game_state_.bb_amount;
}


void Game::UpdateGameState(ActionWithID ac) {

    switch (ac.player_action.action ) {
        case 0:
            game_state_.num_player_in_hand--;
            game_state_.player_status[ac.ID] = 0;
            break;
        case 1:
            game_state_.stack_size[ac.ID] -= ac.player_action.amount;
            game_state_.bet_ring[ac.ID] += ac.player_action.amount;
            break;
        case 2:
            game_state_.aggressor = ac.ID;
            game_state_.raise_amount = ac.player_action.amount - *std::max_element(game_state_.bet_ring,game_state_.bet_ring+9);
            game_state_.stack_size[ac.ID] -= (ac.player_action.amount - game_state_.bet_ring[ac.ID] ) ;
            game_state_.bet_ring[ac.ID] = ac.player_action.amount;
            break;
        default:
            std::cerr << "[ERROR] Unknown player action " << ac.player_action.action << " by " << ac.ID << std::endl;
            std::cerr << "default to fold" << std::endl;
            game_state_.num_player_in_hand--;
            game_state_.player_status[ac.ID] = 0;
    }
    //Update game history game_state_.ActionHistory
    switch (game_state_.current_street) {
        case 0:
            game_state_.action_history.preflop.push_back(ac);
            break;
        case 1:
            game_state_.action_history.flop.push_back(ac);
            break;
        case 2:
            game_state_.action_history.turn.push_back(ac);
            break;
        case 3:
            game_state_.action_history.river.push_back(ac);
            break;
        
    }
    
    
    game_state_.next_player_to_act = FindNextPlayer(game_state_.next_player_to_act);


}

void Game::RemovePlayerCard() {
    for (int i = 0 ; i < game_state_.num_player; i++ )
        players[i]->ResetHoleCards();
}

void Game::CleanCommunityCard() {
    game_state_.community_cards.clear();
}

void Game::ResetGameState() {
    //std::copy(game_state_.stack_size, game_state_.stack_size+9, game_state_.starting_stack_size);

    // Compute bankroll, always rebuy/adjust to 100BB
    for (int iplayer = 0 ; iplayer < 9 ; iplayer++ )
        game_state_.bankroll[iplayer] += game_state_.stack_size[iplayer] - game_state_.starting_stack_size[iplayer];
    
    // Always reset stacksize
    std::copy(game_state_.starting_stack_size, game_state_.starting_stack_size+9, game_state_.stack_size); 

    game_state_.current_street = 0;
    game_state_.pot_size = 0;
    game_state_.num_player_in_hand = game_state_.num_player;
}


ActionWithID Game::VerifyAction(ActionWithID ac, LegalActions legal_actions) {
    if ( ac.player_action.action == 1 ) {
        if (ac.player_action.amount != legal_actions.LegalCall.amount ) {
            std::cerr << "[WARNING] call amount is invalid: " << ac.player_action.amount  \
                      << " ,should be : " <<  legal_actions.LegalCall.amount \
                      << " .Default to fold" << std::endl;
            ac.player_action.amount = 0;
            ac.player_action.action = 0;
        }
    }
    else if ( ac.player_action.action == 2) {
        if ( ac.player_action.amount < legal_actions.LegalMinRaise.amount ) {
            std::cerr << "[WARNING] raise amount is invalid: " << ac.player_action.amount  \
                      << " ,should be at least: " <<  legal_actions.LegalMinRaise.amount \
                      << " .Default to fold" << std::endl; 
            ac.player_action.amount = 0;
            ac.player_action.action = 0;
        }

        if ( ac.player_action.amount > legal_actions.LegalMaxRaise.amount ) {
            std::cerr << "[WARNING] raise amount is invalid: " << ac.player_action.amount  \
                      << " ,should be at most: " <<  legal_actions.LegalMaxRaise.amount  \
                      << " .Default to fold" << std::endl; 
            ac.player_action.amount = 0;
            ac.player_action.action = 0;
        }

    }
    return ac;
}

LegalActions Game::GetAllLegalActions() {
    //Correct ac if it is invalid
    int biggest_bet_amount = *std::max_element( game_state_.bet_ring, game_state_.bet_ring+9 );
    LegalActions legal_actions;
    legal_actions.LegalFold.action = 0;
    legal_actions.LegalFold.amount = 0;

    legal_actions.LegalCall.action = 1;
    legal_actions.LegalCall.amount = std::min ( biggest_bet_amount \
                                    - game_state_.bet_ring[game_state_.next_player_to_act] \
                                    , game_state_.stack_size[game_state_.next_player_to_act] );

    legal_actions.LegalMinRaise.action = 2;
    legal_actions.LegalMinRaise.amount = biggest_bet_amount + game_state_.raise_amount;

    legal_actions.LegalMaxRaise.action = 2;
    legal_actions.LegalMaxRaise.amount = game_state_.stack_size[game_state_.next_player_to_act] ;

    if ( biggest_bet_amount > game_state_.stack_size[game_state_.next_player_to_act] ) {
        legal_actions.LegalMinRaise.amount = -1 ;
        legal_actions.LegalMaxRaise.amount = -1 ;
    }
        
    if ( legal_actions.LegalMinRaise.amount > legal_actions.LegalMaxRaise.amount ) {
        legal_actions.LegalMinRaise.amount = game_state_.stack_size[game_state_.next_player_to_act] ;
        legal_actions.LegalMaxRaise.amount = game_state_.stack_size[game_state_.next_player_to_act] ;        
    }

    return legal_actions;
}

