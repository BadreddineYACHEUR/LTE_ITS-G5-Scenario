#ifndef HYBRIDSERVICE_H_
#define HYBRIDSERVICE_H_

#include "artery/application/ItsG5Service.h"
#include "artery/application/VehicleDataProvider.h"

#include "torch/torch.h"
#include <iostream>
#include <stdio.h>
#include "RLAgentUtils/Network.h"
#include "RLAgentUtils/Memory.h"
#include <string>

class HybridService : public artery::ItsG5Service
{
	public:
	//	Agent(int input_dims, int num_actions, int hidden_dims);
		HybridService();
		void init(int input_dims, int num_actions, int hidden_dims);
		
		void indicate(const vanetza::btp::DataIndication&, omnetpp::cPacket*) override;
		void trigger() override;
		void receiveSignal (cComponent*, omnetpp::simsignal_t, cObject* /*const char**/, cObject*) override;
		void sendToMainApp (omnetpp::cMessage* msg, std::string id);

		// RAT selection Control agent functions  

        int choose_action(torch::Tensor state);
        void store_transition(torch::Tensor state, torch::Tensor new_state, int action, double reward, bool done);

        void replace_target_network();
        void decrement_epsilon();
		
		void learn();


	protected:
		enum V_ROLE {FREE, LEADER, FOLLOWER, JOINER};
    	V_ROLE role = FREE;

		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;
		void CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed);
        void CACCGapControl(std::string vehicle_id, std::string pre_vehicle_id, double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel);
        double squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2);

        // Dup Messages Managment

        std::list <std::string> receivedMessages;

		//Signals

		omnetpp::simsignal_t toMainAppSignal;

		// Platoon managment

		std::string csvFile;
        int messageId = 0;
        int platoonId;
        int platoonSize = 0;
        double leader_speed = 0;
        int message_hits = 0;
        bool is_in_platoon = false;
        std::list <platoonMember> platoonMembers;



	private:
		traci::VehicleController* mVehicleController = nullptr;
		omnetpp::cMessage* m_self_msg;
	

	// RAT selection Control agent parameters 

	private: Net network;
	private: Net target_network;
    private: std::string checkpoint;
    private: double epsilon = 1.0;
    private: double epsilon_min = 0.01;
    private: double epsilon_decay = 5e-7;
    private: float lr = 0.01;
    private: float gamma = 0.99;
    private: torch::optim::Adam optimizer;
    private: int action_space, observation_space;
    
    private: int learn_step_counter = 0;
    private: int replace_target_cnt = 10000;
    private: int batch_size = 32;
    private: int mem_max = 500000;
    
    private: Memory agent_memory;

};

#endif /* HYBRIDSERVICE_H_ */