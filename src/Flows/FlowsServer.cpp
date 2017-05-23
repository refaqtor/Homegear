/* Copyright 2013-2017 Sathya Laufer
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Homegear.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#include "FlowsServer.h"
#include "../GD/GD.h"
#include <homegear-base/BaseLib.h>

namespace Flows
{

FlowsServer::FlowsServer() : IQueue(GD::bl.get(), 2, 1000)
{
	_out.init(GD::bl.get());
	_out.setPrefix("Flows Engine Server: ");

	_shuttingDown = false;
	_stopServer = false;
	_nodeEventsEnabled = false;

	_rpcDecoder = std::unique_ptr<BaseLib::Rpc::RpcDecoder>(new BaseLib::Rpc::RpcDecoder(GD::bl.get(), false, false));
	_rpcEncoder = std::unique_ptr<BaseLib::Rpc::RpcEncoder>(new BaseLib::Rpc::RpcEncoder(GD::bl.get(), true));
	_jsonEncoder = std::unique_ptr<BaseLib::Rpc::JsonEncoder>(new BaseLib::Rpc::JsonEncoder(GD::bl.get()));
	_jsonDecoder = std::unique_ptr<BaseLib::Rpc::JsonDecoder>(new BaseLib::Rpc::JsonDecoder(GD::bl.get()));
	_dummyClientInfo.reset(new BaseLib::RpcClientInfo());

	_rpcMethods.emplace("devTest", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCDevTest()));
	_rpcMethods.emplace("system.getCapabilities", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSystemGetCapabilities()));
	_rpcMethods.emplace("system.listMethods", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSystemListMethods(GD::rpcServers[0].getServer())));
	_rpcMethods.emplace("system.methodHelp", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSystemMethodHelp(GD::rpcServers[0].getServer())));
	_rpcMethods.emplace("system.methodSignature", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSystemMethodSignature(GD::rpcServers[0].getServer())));
	_rpcMethods.emplace("system.multicall", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSystemMulticall(GD::rpcServers[0].getServer())));
	_rpcMethods.emplace("activateLinkParamset", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCActivateLinkParamset()));
	_rpcMethods.emplace("abortEventReset", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCTriggerEvent()));
	_rpcMethods.emplace("addDevice", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCAddDevice()));
	_rpcMethods.emplace("addEvent", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCAddEvent()));
	_rpcMethods.emplace("addLink", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCAddLink()));
	_rpcMethods.emplace("copyConfig", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCCopyConfig()));
	_rpcMethods.emplace("clientServerInitialized", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCClientServerInitialized()));
	_rpcMethods.emplace("createDevice", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCCreateDevice()));
	_rpcMethods.emplace("deleteData", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCDeleteData()));
	_rpcMethods.emplace("deleteDevice", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCDeleteDevice()));
	_rpcMethods.emplace("deleteMetadata", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCDeleteMetadata()));
	_rpcMethods.emplace("deleteNodeData", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCDeleteNodeData()));
	_rpcMethods.emplace("deleteSystemVariable", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCDeleteSystemVariable()));
	_rpcMethods.emplace("enableEvent", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCEnableEvent()));
	_rpcMethods.emplace("getAllConfig", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetAllConfig()));
	_rpcMethods.emplace("getAllMetadata", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetAllMetadata()));
	_rpcMethods.emplace("getAllScripts", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetAllScripts()));
	_rpcMethods.emplace("getAllSystemVariables", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetAllSystemVariables()));
	_rpcMethods.emplace("getAllValues", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetAllValues()));
	_rpcMethods.emplace("getConfigParameter", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetConfigParameter()));
	_rpcMethods.emplace("getData", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetData()));
	_rpcMethods.emplace("getDeviceDescription", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetDeviceDescription()));
	_rpcMethods.emplace("getDeviceInfo", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetDeviceInfo()));
	_rpcMethods.emplace("getEvent", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetEvent()));
	_rpcMethods.emplace("getInstallMode", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetInstallMode()));
	_rpcMethods.emplace("getKeyMismatchDevice", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetKeyMismatchDevice()));
	_rpcMethods.emplace("getLinkInfo", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetLinkInfo()));
	_rpcMethods.emplace("getLinkPeers", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetLinkPeers()));
	_rpcMethods.emplace("getLinks", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetLinks()));
	_rpcMethods.emplace("getMetadata", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetMetadata()));
	_rpcMethods.emplace("getName", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetName()));
	_rpcMethods.emplace("getNodeData", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetNodeData()));
	_rpcMethods.emplace("getPairingMethods", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetPairingMethods()));
	_rpcMethods.emplace("getParamset", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetParamset()));
	_rpcMethods.emplace("getParamsetDescription", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetParamsetDescription()));
	_rpcMethods.emplace("getParamsetId", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetParamsetId()));
	_rpcMethods.emplace("getPeerId", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetPeerId()));
	_rpcMethods.emplace("getServiceMessages", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetServiceMessages()));
	_rpcMethods.emplace("getSystemVariable", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetSystemVariable()));
	_rpcMethods.emplace("getUpdateStatus", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetUpdateStatus()));
	_rpcMethods.emplace("getValue", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetValue()));
	_rpcMethods.emplace("getVariableDescription", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetVariableDescription()));
	_rpcMethods.emplace("getVersion", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCGetVersion()));
	_rpcMethods.emplace("init", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCInit()));
	_rpcMethods.emplace("listBidcosInterfaces", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListBidcosInterfaces()));
	_rpcMethods.emplace("listClientServers", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListClientServers()));
	_rpcMethods.emplace("listDevices", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListDevices()));
	_rpcMethods.emplace("listEvents", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListEvents()));
	_rpcMethods.emplace("listFamilies", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListFamilies()));
	_rpcMethods.emplace("listInterfaces", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListInterfaces()));
	_rpcMethods.emplace("listKnownDeviceTypes", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListKnownDeviceTypes()));
	_rpcMethods.emplace("listTeams", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCListTeams()));
	_rpcMethods.emplace("logLevel", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCLogLevel()));
	_rpcMethods.emplace("ping", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCPing()));
	_rpcMethods.emplace("putParamset", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCPutParamset()));
	_rpcMethods.emplace("removeEvent", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCRemoveEvent()));
	_rpcMethods.emplace("removeLink", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCRemoveLink()));
	_rpcMethods.emplace("reportValueUsage", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCReportValueUsage()));
	_rpcMethods.emplace("rssiInfo", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCRssiInfo()));
	_rpcMethods.emplace("runScript", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCRunScript()));
	_rpcMethods.emplace("searchDevices", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSearchDevices()));
	_rpcMethods.emplace("setData", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetData()));
	_rpcMethods.emplace("setId", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetId()));
	_rpcMethods.emplace("setInstallMode", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetInstallMode()));
	_rpcMethods.emplace("setInterface", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetInterface()));
	_rpcMethods.emplace("setLinkInfo", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetLinkInfo()));
	_rpcMethods.emplace("setMetadata", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetMetadata()));
	_rpcMethods.emplace("setName", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetName()));
	_rpcMethods.emplace("setNodeData", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetNodeData()));
	_rpcMethods.emplace("setNodeVariable", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetNodeVariable()));
	_rpcMethods.emplace("setSystemVariable", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetSystemVariable()));
	_rpcMethods.emplace("setTeam", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetTeam()));
	_rpcMethods.emplace("setValue", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSetValue()));
	_rpcMethods.emplace("subscribePeers", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCSubscribePeers()));
	_rpcMethods.emplace("triggerEvent", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCTriggerEvent()));
	_rpcMethods.emplace("triggerRpcEvent", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCTriggerRpcEvent()));
	_rpcMethods.emplace("unsubscribePeers", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCUnsubscribePeers()));
	_rpcMethods.emplace("updateFirmware", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCUpdateFirmware()));
	_rpcMethods.emplace("writeLog", std::shared_ptr<BaseLib::Rpc::RpcMethod>(new Rpc::RPCWriteLog()));

	_localRpcMethods.insert(std::pair<std::string, std::function<BaseLib::PVariable(PFlowsClientData& clientData, BaseLib::PArray& parameters)>>("registerFlowsClient", std::bind(&FlowsServer::registerFlowsClient, this, std::placeholders::_1, std::placeholders::_2)));
	_localRpcMethods.insert(std::pair<std::string, std::function<BaseLib::PVariable(PFlowsClientData& clientData, BaseLib::PArray& parameters)>>("executePhpNode", std::bind(&FlowsServer::executePhpNode, this, std::placeholders::_1, std::placeholders::_2)));
	_localRpcMethods.insert(std::pair<std::string, std::function<BaseLib::PVariable(PFlowsClientData& clientData, BaseLib::PArray& parameters)>>("nodeEvent", std::bind(&FlowsServer::nodeEvent, this, std::placeholders::_1, std::placeholders::_2)));
}

FlowsServer::~FlowsServer()
{
	if(!_stopServer) stop();
}

void FlowsServer::collectGarbage()
{
	try
	{
		_lastGargabeCollection = GD::bl->hf.getTime();
		std::vector<PFlowsProcess> processesToShutdown;
		{
			std::lock_guard<std::mutex> processGuard(_processMutex);
			bool emptyProcess = false;
			for(std::map<pid_t, PFlowsProcess>::iterator i = _processes.begin(); i != _processes.end(); ++i)
			{
				if(i->second->flowCount() == 0 && i->second->getClientData() && !i->second->getClientData()->closed)
				{
					if(emptyProcess) closeClientConnection(i->second->getClientData());
					else emptyProcess = true;
				}
			}
		}

		std::vector<PFlowsClientData> clientsToRemove;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			try
			{
				for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
				{
					if(i->second->closed) clientsToRemove.push_back(i->second);
				}
			}
			catch(const std::exception& ex)
			{
				_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
			}
			catch(...)
			{
				_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
			}
		}
		for(std::vector<PFlowsClientData>::iterator i = clientsToRemove.begin(); i != clientsToRemove.end(); ++i)
		{
			{
				std::lock_guard<std::mutex> stateGuard(_stateMutex);
				_clients.erase((*i)->id);
			}
			_out.printDebug("Debug: Flows engine client " + std::to_string((*i)->id) + " removed.");
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::getMaxThreadCounts()
{
	try
	{
		_maxThreadCounts.clear();
		std::vector<NodeManager::PNodeInfo> nodeInfo = NodeManager::getNodeInfo();
		for(auto& infoEntry : nodeInfo)
		{
			_maxThreadCounts[infoEntry->nodeName] = infoEntry->maxThreadCount;
		}
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

bool FlowsServer::start()
{
	try
	{
		stop();
		_socketPath = GD::bl->settings.socketPath() + "homegearFE.sock";
		_shuttingDown = false;
		_stopServer = false;
		if(!getFileDescriptor(true)) return false;
		_webroot = GD::bl->settings.flowsPath() + "www/";
		getMaxThreadCounts();
		startQueue(0, GD::bl->settings.flowsProcessingThreadCountServer(), 0, SCHED_OTHER);
		startQueue(1, GD::bl->settings.flowsProcessingThreadCountServer(), 0, SCHED_OTHER);
		GD::bl->threadManager.start(_mainThread, true, &FlowsServer::mainThread, this);
		startFlows();
		return true;
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return false;
}

void FlowsServer::stop()
{
	try
	{
		_shuttingDown = true;
		_stopServer = true;
		_out.printDebug("Debug: Waiting for flows engine server's client threads to finish.");
		closeClientConnections();
		GD::bl->threadManager.join(_mainThread);
		stopQueue(0);
		stopQueue(1);
		unlink(_socketPath.c_str());
	}
	catch(const std::exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void FlowsServer::homegearShuttingDown()
{
	try
	{
		_shuttingDown = true;
		sendShutdown();
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::homegearReloading()
{
	try
	{
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}
		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array());
			sendRequest(*i, "reload", parameters);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::processKilled(pid_t pid, int32_t exitCode, int32_t signal, bool coreDumped)
{
	try
	{
		std::shared_ptr<FlowsProcess> process;

		{
			std::lock_guard<std::mutex> processGuard(_processMutex);
			std::map<pid_t, PFlowsProcess>::iterator processIterator = _processes.find(pid);
			if(processIterator != _processes.end())
			{
				process = processIterator->second;
				closeClientConnection(process->getClientData());
				_processes.erase(processIterator);
			}
		}

		if(process)
		{
			if(signal != -1 && signal != 15) _out.printCritical("Critical: Client process with pid " + std::to_string(pid) + " was killed with signal " + std::to_string(signal) + '.');
			else if(exitCode != 0) _out.printError("Error: Client process with pid " + std::to_string(pid) + " exited with code " + std::to_string(exitCode) + '.');
			else _out.printInfo("Info: Client process with pid " + std::to_string(pid) + " exited with code " + std::to_string(exitCode) + '.');

			if(signal != -1) exitCode = -32500;
			process->invokeFlowFinished(exitCode);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::nodeOutput(std::string nodeId, uint32_t index, BaseLib::PVariable message)
{
	try
	{
		PFlowsClientData clientData;
		int32_t clientId = 0;
		{
			std::lock_guard<std::mutex> nodeClientIdMapGuard(_nodeClientIdMapMutex);
			auto nodeClientIdIterator = _nodeClientIdMap.find(nodeId);
			if(nodeClientIdIterator == _nodeClientIdMap.end()) return;
			clientId = nodeClientIdIterator->second;
		}
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			auto clientIterator = _clients.find(clientId);
			if(clientIterator == _clients.end()) return;
			clientData = clientIterator->second;
		}

		BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
		parameters->reserve(3);
		parameters->push_back(std::make_shared<BaseLib::Variable>(nodeId));
		parameters->push_back(std::make_shared<BaseLib::Variable>(index));
		parameters->push_back(message);
		sendRequest(clientData, "nodeOutput", parameters);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::setNodeVariable(std::string nodeId, std::string topic, BaseLib::PVariable value)
{
	try
	{
		PFlowsClientData clientData;
		int32_t clientId = 0;
		{
			std::lock_guard<std::mutex> nodeClientIdMapGuard(_nodeClientIdMapMutex);
			auto nodeClientIdIterator = _nodeClientIdMap.find(nodeId);
			if(nodeClientIdIterator == _nodeClientIdMap.end()) return;
			clientId = nodeClientIdIterator->second;
		}
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			auto clientIterator = _clients.find(clientId);
			if(clientIterator == _clients.end()) return;
			clientData = clientIterator->second;
		}

		BaseLib::PArray parameters = std::make_shared<BaseLib::Array>();
		parameters->reserve(3);
		parameters->push_back(std::make_shared<BaseLib::Variable>(nodeId));
		parameters->push_back(std::make_shared<BaseLib::Variable>(topic));
		parameters->push_back(value);
		sendRequest(clientData, "setNodeVariable", parameters);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::enableNodeEvents()
{
	try
	{
		if(_shuttingDown) return;
		_out.printInfo("Info: Enabling node events...");
		_nodeEventsEnabled = true;
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}

		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array());
			BaseLib::PVariable response = sendRequest(*i, "enableNodeEvents", parameters);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::disableNodeEvents()
{
	try
	{
		if(_shuttingDown) return;
		_out.printInfo("Info: Disabling node events...");
		_nodeEventsEnabled = false;
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}

		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array());
			BaseLib::PVariable response = sendRequest(*i, "disableNodeEvents", parameters);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

std::set<std::string> FlowsServer::insertSubflows(BaseLib::PVariable& subflowNode, std::unordered_map<std::string, BaseLib::PVariable>& subflowInfos, std::unordered_map<std::string, BaseLib::PVariable>& flowNodes, std::unordered_map<std::string, BaseLib::PVariable>& subflowNodes, std::set<std::string>& flowNodeIds, std::set<std::string>& allNodeIds)
{
	try
	{
		std::set<std::string> subsubflows;
		std::vector<std::pair<std::string, BaseLib::PVariable>> nodesToAdd;

		std::string thisSubflowId = subflowNode->structValue->at("id")->stringValue;
		std::string subflowIdPrefix = thisSubflowId + ':';
		std::string subflowId = subflowNode->structValue->at("type")->stringValue.substr(8);
		auto subflowInfoIterator = subflowInfos.find(subflowId);
		if(subflowInfoIterator == subflowInfos.end())
		{
			GD::out.printError("Error: Could not find subflow info with for subflow with id " + subflowId);
			return std::set<std::string>();
		}

		//Copy subflow, prefix all subflow node IDs and wires with subflow ID and identify subsubflows
		std::unordered_map<std::string, BaseLib::PVariable> subflow;
		nodesToAdd.reserve(subflowNodes.size());
		for(auto& subflowElement : subflowNodes)
		{
			BaseLib::PVariable subflowNode = std::make_shared<BaseLib::Variable>();
			*subflowNode = *subflowElement.second;
			subflowNode->structValue->at("id")->stringValue = subflowIdPrefix + subflowNode->structValue->at("id")->stringValue;
			allNodeIds.emplace(subflowNode->structValue->at("id")->stringValue);
			flowNodeIds.emplace(subflowNode->structValue->at("id")->stringValue);

			auto& subflowNodeType = subflowNode->structValue->at("type")->stringValue;
			if(subflowNodeType.compare(0, 8, "subflow:") == 0)
			{
				subsubflows.emplace(subflowNode->structValue->at("id")->stringValue);
			}

			auto& subflowNodeWires = subflowNode->structValue->at("wires");
			for(auto& output : *subflowNodeWires->arrayValue)
			{
				for(auto& wire : *output->arrayValue)
				{
					auto idIterator = wire->structValue->find("id");
					if(idIterator == wire->structValue->end()) continue;
					idIterator->second->stringValue = subflowIdPrefix + wire->stringValue;
				}
			}

			subflow.emplace(subflowElement.first, subflowNode);
			nodesToAdd.push_back(std::make_pair(subflowNode->structValue->at("id")->stringValue, subflowNode));
		}

		//Find output node and redirect it to the subflow output
		std::multimap<uint32_t, std::pair<uint32_t, std::string>> passthroughWires;
		auto wiresOutIterator = subflowInfoIterator->second->structValue->find("out");
		if(wiresOutIterator != subflowInfoIterator->second->structValue->end())
		{
			for(uint32_t outputIndex = 0; outputIndex < wiresOutIterator->second->arrayValue->size(); ++outputIndex)
			{
				auto& element = wiresOutIterator->second->arrayValue->at(outputIndex);
				auto innerIterator = element->structValue->find("wires");
				if(innerIterator == element->structValue->end()) continue;
				for(auto& wireIterator : *innerIterator->second->arrayValue)
				{
					auto wireIdIterator = wireIterator->structValue->find("id");
					if(wireIdIterator == wireIterator->structValue->end()) continue;

					auto wirePortIterator = wireIterator->structValue->find("port");
					if(wirePortIterator == wireIterator->structValue->end()) continue;

					if(wirePortIterator->second->type == BaseLib::VariableType::tString)
					{
						wirePortIterator->second->integerValue = BaseLib::Math::getNumber(wirePortIterator->second->stringValue);
					}

					auto& targetNodeWires = subflowNode->structValue->at("wires");
					if(outputIndex >= targetNodeWires->arrayValue->size()) continue;

					auto& targetNodeWireOutput = targetNodeWires->arrayValue->at(outputIndex);

					if(wireIdIterator->second->stringValue == subflowId)
					{
						for(auto targetNodeWire : *targetNodeWireOutput->arrayValue)
						{
							auto targetNodeWireIdIterator = targetNodeWire->structValue->find("id");
							if(targetNodeWireIdIterator == targetNodeWire->structValue->end()) continue;

							uint32_t port = 0;
							auto targetNodeWirePortIterator = targetNodeWire->structValue->find("port");
							if(targetNodeWirePortIterator != targetNodeWire->structValue->end())
							{
								if(targetNodeWirePortIterator->second->type == BaseLib::VariableType::tString) port = BaseLib::Math::getNumber(targetNodeWirePortIterator->second->stringValue);
								else port = targetNodeWirePortIterator->second->integerValue;
							}

							passthroughWires.emplace(wirePortIterator->second->integerValue, std::make_pair(port, targetNodeWireIdIterator->second->stringValue));
						}
						continue;
					}

					auto sourceNodeIterator = subflow.find(wireIdIterator->second->stringValue);
					if(sourceNodeIterator == subflow.end()) continue;

					auto sourceNodeWires = sourceNodeIterator->second->structValue->at("wires");
					while((signed)sourceNodeWires->arrayValue->size() < wirePortIterator->second->integerValue + 1) sourceNodeWires->arrayValue->push_back(std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray));

					*sourceNodeWires->arrayValue->at(wirePortIterator->second->integerValue) = *targetNodeWires->arrayValue->at(outputIndex);
				}
			}
		}

		//Redirect all subflow inputs to subflow nodes
		auto wiresInIterator = subflowInfoIterator->second->structValue->find("in");
		if(wiresInIterator != subflowInfoIterator->second->structValue->end())
		{
			std::vector<BaseLib::PArray> wiresIn;
			wiresIn.reserve(wiresInIterator->second->arrayValue->size());
			for(uint32_t i = 0; i < wiresInIterator->second->arrayValue->size(); i++)
			{
				wiresIn.push_back(std::make_shared<BaseLib::Array>());
				auto wiresInWiresIterator = wiresInIterator->second->arrayValue->at(i)->structValue->find("wires");
				if(wiresInWiresIterator != wiresInIterator->second->arrayValue->at(i)->structValue->end())
				{
					wiresIn.back()->reserve(wiresInIterator->second->arrayValue->at(i)->structValue->size());
					for(auto& wiresInWire : *wiresInWiresIterator->second->arrayValue)
					{
						auto wiresInWireIdIterator = wiresInWire->structValue->find("id");
						if(wiresInWireIdIterator == wiresInWire->structValue->end()) continue;

						uint32_t port = 0;
						auto wiresInWirePortIterator = wiresInWire->structValue->find("port");
						if(wiresInWirePortIterator != wiresInWire->structValue->end())
						{
							if(wiresInWirePortIterator->second->type == BaseLib::VariableType::tString) port = BaseLib::Math::getNumber(wiresInWirePortIterator->second->stringValue);
							else port = wiresInWirePortIterator->second->integerValue;
						}

						BaseLib::PVariable entry = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
						entry->structValue->emplace("id", std::make_shared<BaseLib::Variable>(subflowIdPrefix + wiresInWireIdIterator->second->stringValue));
						entry->structValue->emplace("port", std::make_shared<BaseLib::Variable>(port));
						wiresIn.back()->push_back(entry);
					}
				}
			}

			for(auto& node2 : flowNodes)
			{
				auto& node2Wires = node2.second->structValue->at("wires");
				for(auto& node2WiresOutput : *node2Wires->arrayValue)
				{
					bool rebuildArray = false;
					for(auto& node2Wire : *node2WiresOutput->arrayValue)
					{
						auto node2WireIdIterator = node2Wire->structValue->find("id");
						if(node2WireIdIterator == node2Wire->structValue->end()) continue;

						if(node2WireIdIterator->second->stringValue == thisSubflowId)
						{
							rebuildArray = true;
							break;
						}
					}
					if(rebuildArray)
					{
						BaseLib::PArray wireArray = std::make_shared<BaseLib::Array>();
						for(auto& node2Wire : *node2WiresOutput->arrayValue)
						{
							auto node2WireIdIterator = node2Wire->structValue->find("id");
							if(node2WireIdIterator == node2Wire->structValue->end()) continue;

							uint32_t port = 0;
							auto node2WirePortIterator = node2Wire->structValue->find("port");
							if(node2WirePortIterator != node2Wire->structValue->end())
							{
								if(node2WirePortIterator->second->type == BaseLib::VariableType::tString) port = BaseLib::Math::getNumber(node2WirePortIterator->second->stringValue);
								else port = node2WirePortIterator->second->integerValue;
							}

							if(node2WireIdIterator->second->stringValue != thisSubflowId) wireArray->push_back(node2Wire);
							else wireArray->insert(wireArray->end(), wiresIn.at(port)->begin(), wiresIn.at(port)->end());

							auto passthroughWiresIterators = passthroughWires.equal_range(port);
							for(auto passthroughWiresIterator = passthroughWiresIterators.first; passthroughWiresIterator != passthroughWiresIterators.second; passthroughWiresIterator++)
							{
								BaseLib::PVariable entry = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
								entry->structValue->emplace("id", std::make_shared<BaseLib::Variable>(passthroughWiresIterator->second.second));
								entry->structValue->emplace("port", std::make_shared<BaseLib::Variable>(passthroughWiresIterator->second.first));
								wireArray->push_back(entry);
							}
						}
						node2WiresOutput->arrayValue = wireArray;
					}
				}
			}
		}
		flowNodes.insert(nodesToAdd.begin(), nodesToAdd.end());
		return subsubflows;
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return std::set<std::string>();
}

void FlowsServer::startFlows()
{
	try
	{
		std::string flowsfile = GD::bl->settings.flowsPath() + "data/flows.json";
		if(!GD::bl->io.fileExists(flowsfile)) return;
		std::string rawFlows = GD::bl->io.getFileContent(flowsfile);
		if(BaseLib::HelperFunctions::trim(rawFlows).empty()) return;

		//{{{ Filter all nodes and assign it to flows
			BaseLib::PVariable flows = _jsonDecoder->decode(rawFlows);
			std::unordered_map<std::string, BaseLib::PVariable> subflowInfos;
			std::unordered_map<std::string, std::unordered_map<std::string, BaseLib::PVariable>> flowNodes;
			std::unordered_map<std::string, std::set<std::string>> subflowNodeIds;
			std::unordered_map<std::string, std::set<std::string>> nodeIds;
			std::set<std::string> allNodeIds;
			for(auto& element : *flows->arrayValue)
			{
				auto idIterator = element->structValue->find("id");
				if(idIterator == element->structValue->end())
				{
					GD::out.printError("Error: Flow element has no id.");
					continue;
				}

				auto typeIterator = element->structValue->find("type");
				if(typeIterator == element->structValue->end()) continue;
				if(typeIterator->second->stringValue == "comment") continue;
				if(typeIterator->second->stringValue == "subflow")
				{
					subflowInfos.emplace(idIterator->second->stringValue, element);
					continue;
				}

				auto wiresIterator = element->structValue->find("wires");
				if(wiresIterator == element->structValue->end()) continue; //Element is no node

				auto zIterator = element->structValue->find("z");
				if(zIterator == element->structValue->end()) continue;

				if(allNodeIds.find(idIterator->second->stringValue) != allNodeIds.end())
				{
					GD::out.printError("Error: Flow element is defined twice: " + idIterator->second->stringValue + ". At least one of the flows is not working correctly.");
					continue;
				}

				allNodeIds.emplace(idIterator->second->stringValue);
				nodeIds[zIterator->second->stringValue].emplace(idIterator->second->stringValue);
				flowNodes[zIterator->second->stringValue].emplace(idIterator->second->stringValue, element);
				if(typeIterator->second->stringValue.compare(0, 8, "subflow:") == 0) subflowNodeIds[zIterator->second->stringValue].emplace(idIterator->second->stringValue);
			}
		//}}}

		//{{{ Insert subflows
			for(auto& subflowNodeIdsInner : subflowNodeIds)
			{
				if(subflowInfos.find(subflowNodeIdsInner.first) != subflowInfos.end()) continue; //Don't process nodes of subflows

				for(auto subflowNodeId : subflowNodeIdsInner.second)
				{
					std::set<std::string> processedSubflows;

					std::set<std::string> subsubflows;
					subsubflows.emplace(subflowNodeId);
					while(!subsubflows.empty())
					{
						std::set<std::string> newSubsubflows;
						for(auto& subsubflowNodeId : subsubflows)
						{
							auto& innerFlowNodes = flowNodes.at(subflowNodeIdsInner.first);
							auto subflowNodeIterator = innerFlowNodes.find(subsubflowNodeId);
							if(subflowNodeIterator == innerFlowNodes.end())
							{
								_out.printError("Error: Could not find subflow node ID \"" + subsubflowNodeId + "\" in flow with ID \"" + subflowNodeIdsInner.first + "\".");
								continue;
							}
							std::string subflowId = subflowNodeIterator->second->structValue->at("type")->stringValue.substr(8);
							auto subflowNodesIterator = flowNodes.find(subflowId);
							if(subflowNodesIterator == flowNodes.end())
							{
								_out.printError("Error: Could not find subflow with ID \"" + subflowId + "\" in flows.");
								continue;
							}
							std::set<std::string> returnedSubsubflows = insertSubflows(subflowNodeIterator->second, subflowInfos, innerFlowNodes, subflowNodesIterator->second, nodeIds.at(subflowNodeIdsInner.first), allNodeIds);
							processedSubflows.emplace(subsubflowNodeId);
							bool abort = false;
							for(auto& returnedSubsubflow : returnedSubsubflows)
							{
								if(processedSubflows.find(returnedSubsubflow) != processedSubflows.end())
								{
									_out.printError("Error: Subflow recursion detected. Aborting processing subflows for flow with ID: " + subflowNodeIdsInner.first);
									abort = true;
									break;
								}
								newSubsubflows.emplace(returnedSubsubflow);
							}
							if(abort)
							{
								subsubflows.clear();
								break;
							}
						}
						subsubflows = newSubsubflows;
					}
				}
			}
		//}}}

		for(auto& element : flowNodes)
		{
			if(subflowInfos.find(element.first) != subflowInfos.end()) continue;
			BaseLib::PVariable flow = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
			flow->arrayValue->reserve(element.second.size());
			uint32_t maxThreadCount = 0;
			for(auto& node : element.second)
			{
				if(!node.second) continue;
				flow->arrayValue->push_back(node.second);
				auto typeIterator = node.second->structValue->find("type");
				if(typeIterator != node.second->structValue->end())
				{
					if(typeIterator->second->stringValue.compare(0, 8, "subflow:") == 0) continue;
					auto threadCountIterator = _maxThreadCounts.find(typeIterator->second->stringValue);
					if(threadCountIterator == _maxThreadCounts.end())
					{
						GD::out.printError("Error: Could not determine maximum thread count of node \"" + typeIterator->second->stringValue + "\". Node is unknown.");
						continue;
					}
					maxThreadCount += threadCountIterator->second;
				}
				else GD::out.printError("Error: Could not determine maximum thread count of node. No key \"type\".");
			}
			PFlowInfoServer flowInfo = std::make_shared<FlowInfoServer>();
			flowInfo->maxThreadCount = maxThreadCount;
			flowInfo->flow = flow;
			startFlow(flowInfo, nodeIds[element.first]);
		}

		std::set<std::string> nodeData = GD::bl->db->getAllNodeDataNodes();
		std::string dataKey;
		for(auto nodeId : nodeData)
		{
			if(allNodeIds.find(nodeId) == allNodeIds.end()) GD::bl->db->deleteNodeData(nodeId, dataKey);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::sendShutdown()
{
	try
	{
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}
		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array());
			sendRequest(*i, "shutdown", parameters);
		}

		int32_t i = 0;
		while(_clients.size() > 0 && i < 60)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			collectGarbage();
			i++;
		}

		std::lock_guard<std::mutex> nodeClientIdMapGuard(_nodeClientIdMapMutex);
		_nodeClientIdMap.clear();
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::closeClientConnections()
{
	try
	{
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				clients.push_back(i->second);
				closeClientConnection(i->second);
				std::lock_guard<std::mutex> processGuard(_processMutex);
				std::map<pid_t, PFlowsProcess>::iterator processIterator = _processes.find(i->second->pid);
				if(processIterator != _processes.end())
				{
					processIterator->second->invokeFlowFinished(-32501);
					_processes.erase(processIterator);
				}
			}
		}

		while(_clients.size() > 0)
		{
			for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
			{
				(*i)->requestConditionVariable.notify_all();
			}
			collectGarbage();
			if(_clients.size() > 0) std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::restartFlows()
{
	try
	{
		std::lock_guard<std::mutex> restartFlowsGuard(_restartFlowsMutex);
		sendShutdown();
		closeClientConnections();
		getMaxThreadCounts();
		startFlows();
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

std::string FlowsServer::handleGet(std::string& path, BaseLib::Http& http, std::string& responseEncoding)
{
	try
	{
		std::string contentString;
		if(path.compare(0, 14, "flows/locales/") == 0)
		{
			std::string ending = path.substr(path.size() - 4, 4);
			if(ending == ".hni")
			{
				auto pathPair = BaseLib::HelperFunctions::splitFirst(path.substr(14), '/');
				std::string localePath = _bl->settings.flowsPath() + "nodes/" + pathPair.first + "/locales/";
				std::string language = "en-US";
				auto fieldsIterator = http.getHeader().fields.find("accept-language");
				if(fieldsIterator != http.getHeader().fields.end())
				{
					std::pair<std::string, std::string> languagePair = BaseLib::HelperFunctions::splitFirst(fieldsIterator->second, ';');
					std::vector<std::string> languages = BaseLib::HelperFunctions::splitAll(languagePair.first, ',');
					for(auto& l : languages)
					{
						if(GD::bl->io.directoryExists(localePath + l))
						{
							language = l;
							break;
						}
					}
				}
				path = localePath + language + '/' + pathPair.second.substr(0, pathPair.second.size() - 4);
			}
			else
			{
				std::string localePath = _webroot + "static/locales/";
				std::string language = "en-US";
				auto fieldsIterator = http.getHeader().fields.find("accept-language");
				if(fieldsIterator != http.getHeader().fields.end())
				{
					std::pair<std::string, std::string> languagePair = BaseLib::HelperFunctions::splitFirst(fieldsIterator->second, ';');
					std::vector<std::string> languages = BaseLib::HelperFunctions::splitAll(languagePair.first, ',');
					for(auto& l : languages)
					{
						if(GD::bl->io.directoryExists(localePath + l))
						{
							language = l;
							break;
						}
					}
				}
				path = path.substr(13);
				path = localePath + language + path;
			}
			if(GD::bl->io.fileExists(path)) contentString = GD::bl->io.getFileContent(path);
			responseEncoding = "application/json";
		}
		else if (path == "flows/flows")
		{
			std::vector<char> fileContent = _bl->io.getBinaryFileContent(_bl->settings.flowsPath() + "data/flows.json");
			std::vector<char> md5;
			BaseLib::Security::Hash::md5(fileContent, md5);
			std::string md5String = BaseLib::HelperFunctions::getHexString(md5);
			BaseLib::HelperFunctions::toLower(md5String);

			BaseLib::PVariable flowsJson = _jsonDecoder->decode(fileContent);
			BaseLib::PVariable responseJson = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
			responseJson->structValue->emplace("flows", flowsJson);
			responseJson->structValue->emplace("rev", std::make_shared<BaseLib::Variable>(md5String));
			_jsonEncoder->encode(responseJson, contentString);
			responseEncoding = "application/json";
		}
		else if (path == "flows/settings" || path == "flows/library/flows" || path == "flows/debug/view/debug-utils.js")
		{
			path = _webroot + "static/" + path.substr(6);
			if(GD::bl->io.fileExists(path)) contentString = GD::bl->io.getFileContent(path);
			responseEncoding = "application/json";
		}
		else if(path == "flows/nodes")
		{
			path = _webroot + "static/" + path.substr(6);

			std::vector<NodeManager::PNodeInfo> nodeInfo = NodeManager::getNodeInfo();
			if(http.getHeader().fields["accept"] == "text/html")
			{
				responseEncoding = "text/html";
				for(auto& infoEntry : nodeInfo)
				{
					contentString += infoEntry->frontendCode;
				}
			}
			else
			{
				responseEncoding = "application/json";
				BaseLib::PVariable frontendNodeList = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
				for(auto& infoEntry : nodeInfo)
				{
					BaseLib::PVariable nodeListEntry = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tStruct);
					nodeListEntry->structValue->emplace("id", std::make_shared<BaseLib::Variable>(infoEntry->nodeId));
					nodeListEntry->structValue->emplace("name", std::make_shared<BaseLib::Variable>(infoEntry->readableName));
					nodeListEntry->structValue->emplace("types", std::make_shared<BaseLib::Variable>(BaseLib::PArray(new BaseLib::Array{ std::make_shared<BaseLib::Variable>(infoEntry->nodeName) })));
					nodeListEntry->structValue->emplace("enabled", std::make_shared<BaseLib::Variable>(true));
					nodeListEntry->structValue->emplace("local", std::make_shared<BaseLib::Variable>(false));
					nodeListEntry->structValue->emplace("module", std::make_shared<BaseLib::Variable>(infoEntry->nodeName));
					nodeListEntry->structValue->emplace("version", std::make_shared<BaseLib::Variable>(infoEntry->version));
					frontendNodeList->arrayValue->push_back(nodeListEntry);
				}
				_jsonEncoder->encode(frontendNodeList, contentString);
			}
		}
		else if(path.compare(0, 6, "flows/") == 0 && path != "flows/index.php" && path != "flows/signin.php")
		{
			path = _webroot + path.substr(6);
			if(GD::bl->io.fileExists(path)) contentString = GD::bl->io.getFileContent(path);

			std::string ending = "";
			int32_t pos = path.find_last_of('.');
			if(pos != (signed)std::string::npos && (unsigned)pos < path.size() - 1) ending = path.substr(pos + 1);
			GD::bl->hf.toLower(ending);
			responseEncoding = http.getMimeType(ending);
			if(responseEncoding.empty()) responseEncoding = "application/octet-stream";
		}
		return contentString;
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return "";
}

std::string FlowsServer::handlePost(std::string& path, BaseLib::Http& http, std::string& responseEncoding)
{
	try
	{
		if (path == "flows/flows" && http.getHeader().contentType == "application/json" && !http.getContent().empty())
		{
			BaseLib::PVariable json = _jsonDecoder->decode(http.getContent());
			auto flowsIterator = json->structValue->find("flows");
			if (flowsIterator == json->structValue->end()) return "";

			std::vector<char> flows;
			_jsonEncoder->encode(flowsIterator->second, flows);

			{
				std::lock_guard<std::mutex> flowsFileGuard(_flowsFileMutex);
				std::string filename = _bl->settings.flowsPath() + "data/flows.json";
				_bl->io.writeFile(filename, flows, flows.size());
			}

			responseEncoding = "application/json";
			std::vector<char> md5;
			BaseLib::Security::Hash::md5(flows, md5);
			std::string md5String = BaseLib::HelperFunctions::getHexString(md5);
			BaseLib::HelperFunctions::toLower(md5String);

			restartFlows();

			return "{\"rev\": \"" + md5String + "\"}";
		}
	}
	catch (const std::exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch (BaseLib::Exception& ex)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch (...)
	{
		_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return "";
}

uint32_t FlowsServer::flowCount()
{
	try
	{
		if(_shuttingDown) return 0;
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}

		uint32_t count = 0;
		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array());
			BaseLib::PVariable response = sendRequest(*i, "flowCount", parameters);
			count += response->integerValue;
		}
		return count;
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return 0;
}

void FlowsServer::broadcastEvent(uint64_t id, int32_t channel, std::shared_ptr<std::vector<std::string>> variables, BaseLib::PArray values)
{
	try
	{
		if(_shuttingDown) return;
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}

		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array{BaseLib::PVariable(new BaseLib::Variable(id)), BaseLib::PVariable(new BaseLib::Variable(channel)), BaseLib::PVariable(new BaseLib::Variable(*variables)), BaseLib::PVariable(new BaseLib::Variable(values))});
			std::shared_ptr<BaseLib::IQueueEntry> queueEntry = std::make_shared<QueueEntry>(*i, "broadcastEvent", parameters);
			enqueue(1, queueEntry);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::broadcastNewDevices(BaseLib::PVariable deviceDescriptions)
{
	try
	{
		if(_shuttingDown) return;
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}

		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array{deviceDescriptions});
			std::shared_ptr<BaseLib::IQueueEntry> queueEntry = std::make_shared<QueueEntry>(*i, "broadcastNewDevices", parameters);
			enqueue(1, queueEntry);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::broadcastDeleteDevices(BaseLib::PVariable deviceInfo)
{
	try
	{
		if(_shuttingDown) return;
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}

		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array{deviceInfo});
			std::shared_ptr<BaseLib::IQueueEntry> queueEntry = std::make_shared<QueueEntry>(*i, "broadcastDeleteDevices", parameters);
			enqueue(1, queueEntry);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::broadcastUpdateDevice(uint64_t id, int32_t channel, int32_t hint)
{
	try
	{
		if(_shuttingDown) return;
		std::vector<PFlowsClientData> clients;
		{
			std::lock_guard<std::mutex> stateGuard(_stateMutex);
			for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(i->second->closed) continue;
				clients.push_back(i->second);
			}
		}

		for(std::vector<PFlowsClientData>::iterator i = clients.begin(); i != clients.end(); ++i)
		{
			BaseLib::PArray parameters(new BaseLib::Array{BaseLib::PVariable(new BaseLib::Variable(id)), BaseLib::PVariable(new BaseLib::Variable(channel)), BaseLib::PVariable(new BaseLib::Variable(hint))});
			std::shared_ptr<BaseLib::IQueueEntry> queueEntry = std::make_shared<QueueEntry>(*i, "broadcastUpdateDevice", parameters);
			enqueue(1, queueEntry);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::closeClientConnection(PFlowsClientData client)
{
	try
	{
		if(!client) return;
		GD::bl->fileDescriptorManager.shutdown(client->fileDescriptor);
		client->closed = true;
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry>& entry)
{
	try
	{
		std::shared_ptr<QueueEntry> queueEntry;
		queueEntry = std::dynamic_pointer_cast<QueueEntry>(entry);
		if(!queueEntry || queueEntry->clientData->closed) return;

		if(index == 0 && queueEntry->type == QueueEntry::QueueEntryType::defaultType)
		{
			if(queueEntry->isRequest)
			{
				std::string methodName;
				BaseLib::PArray parameters = _rpcDecoder->decodeRequest(queueEntry->packet, methodName);

				if(parameters->size() != 3)
				{
					_out.printError("Error: Wrong parameter count while calling method " + methodName);
					return;
				}

				std::map<std::string, std::function<BaseLib::PVariable(PFlowsClientData& clientData, BaseLib::PArray& parameters)>>::iterator localMethodIterator = _localRpcMethods.find(methodName);
				if(localMethodIterator != _localRpcMethods.end())
				{
					if(GD::bl->debugLevel >= 4)
					{
						_out.printDebug("Info: Client number " + std::to_string(queueEntry->clientData->id) + " is calling RPC method: " + methodName);
						if(GD::bl->debugLevel >= 5)
						{
							for(BaseLib::Array::iterator i = parameters->at(2)->arrayValue->begin(); i != parameters->at(2)->arrayValue->end(); ++i)
							{
								(*i)->print(true, false);
							}
						}
					}
					BaseLib::PVariable result = localMethodIterator->second(queueEntry->clientData, parameters->at(2)->arrayValue);
					if(GD::bl->debugLevel >= 5)
					{
						_out.printDebug("Response: ");
						result->print(true, false);
					}

					sendResponse(queueEntry->clientData, parameters->at(0), parameters->at(1), result);
					return;
				}

				std::map<std::string, std::shared_ptr<BaseLib::Rpc::RpcMethod>>::iterator methodIterator = _rpcMethods.find(methodName);
				if(methodIterator == _rpcMethods.end())
				{
					BaseLib::PVariable result = GD::ipcServer->callRpcMethod(methodName, parameters->at(2)->arrayValue);
					sendResponse(queueEntry->clientData, parameters->at(0), parameters->at(1), result);
					return;
				}

				if(GD::bl->debugLevel >= 4)
				{
					_out.printInfo("Info: Client number " + std::to_string(queueEntry->clientData->id) + " is calling RPC method: " + methodName + " Parameters:");
					for(std::vector<BaseLib::PVariable>::iterator i = parameters->at(2)->arrayValue->begin(); i != parameters->at(2)->arrayValue->end(); ++i)
					{
						(*i)->print(true, false);
					}
				}
				BaseLib::PVariable result = _rpcMethods.at(methodName)->invoke(_dummyClientInfo, parameters->at(2)->arrayValue);
				if(GD::bl->debugLevel >= 5)
				{
					_out.printDebug("Response: ");
					result->print(true, false);
				}

				sendResponse(queueEntry->clientData, parameters->at(0), parameters->at(1), result);
			}
			else
			{
				BaseLib::PVariable response = _rpcDecoder->decodeResponse(queueEntry->packet);
				int32_t packetId = response->arrayValue->at(0)->integerValue;

				{
					std::lock_guard<std::mutex> responseGuard(queueEntry->clientData->rpcResponsesMutex);
					auto responseIterator = queueEntry->clientData->rpcResponses.find(packetId);
					if(responseIterator != queueEntry->clientData->rpcResponses.end())
					{
						PFlowsResponse element = responseIterator->second;
						if(element)
						{
							element->response = response;
							element->packetId = packetId;
							element->finished = true;
						}
					}
				}
				queueEntry->clientData->requestConditionVariable.notify_one();
			}
		}
		else if(index == 1 && queueEntry->type == QueueEntry::QueueEntryType::broadcast) //Second queue for sending packets. Response is processed by first queue
		{
			BaseLib::PVariable response = sendRequest(queueEntry->clientData, queueEntry->methodName, queueEntry->parameters);
			if(response->errorStruct)
			{
				_out.printError("Error calling \"" + queueEntry->methodName + "\" on client " + std::to_string(queueEntry->clientData->id) + ": " + response->structValue->at("faultString")->stringValue);
			}
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

BaseLib::PVariable FlowsServer::send(PFlowsClientData& clientData, std::vector<char>& data)
{
	try
	{
		int32_t totallySentBytes = 0;
		std::lock_guard<std::mutex> sendGuard(clientData->sendMutex);
		while (totallySentBytes < (signed)data.size())
		{
			int32_t sentBytes = ::send(clientData->fileDescriptor->descriptor, &data.at(0) + totallySentBytes, data.size() - totallySentBytes, MSG_NOSIGNAL);
			if(sentBytes <= 0)
			{
				if(errno == EAGAIN) continue;
				GD::out.printError("Could not send data to client: " + std::to_string(clientData->fileDescriptor->descriptor));
				return BaseLib::Variable::createError(-32500, "Unknown application error.");
			}
			totallySentBytes += sentBytes;
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return BaseLib::PVariable(new BaseLib::Variable());
}

BaseLib::PVariable FlowsServer::sendRequest(PFlowsClientData& clientData, std::string methodName, BaseLib::PArray& parameters)
{
	try
	{
		int32_t packetId;
		{
			std::lock_guard<std::mutex> packetIdGuard(_packetIdMutex);
			packetId = _currentPacketId++;
		}
		BaseLib::PArray array(new BaseLib::Array{ BaseLib::PVariable(new BaseLib::Variable(packetId)), BaseLib::PVariable(new BaseLib::Variable(parameters)) });
		std::vector<char> data;
		_rpcEncoder->encodeRequest(methodName, array, data);

		PFlowsResponse response;
		{
			std::lock_guard<std::mutex> responseGuard(clientData->rpcResponsesMutex);
			auto result = clientData->rpcResponses.emplace(packetId, std::make_shared<FlowsResponse>());
			if(result.second) response = result.first->second;
		}
		if(!response)
		{
			_out.printError("Critical: Could not insert response struct into map.");
			return BaseLib::Variable::createError(-32500, "Unknown application error.");
		}

		BaseLib::PVariable result = send(clientData, data);
		if(result->errorStruct)
		{
			std::lock_guard<std::mutex> responseGuard(clientData->rpcResponsesMutex);
			clientData->rpcResponses.erase(packetId);
			return result;
		}

		int32_t i = 0;
		std::unique_lock<std::mutex> waitLock(clientData->waitMutex);
		while(!clientData->requestConditionVariable.wait_for(waitLock, std::chrono::milliseconds(1000), [&]{
			return response->finished || clientData->closed || _stopServer;
		}))
		{
			i++;
			if(i == 60)
			{
				_out.printError("Error: Flows client with process ID " + std::to_string(clientData->pid) + " is not responding... Killing it.");
				kill(clientData->pid, 9);
				processKilled(clientData->pid, -1, 9, false); //Needs to be called manually
				break;
			}
		}

		if(!response->finished || response->response->arrayValue->size() != 2 || response->packetId != packetId)
		{
			_out.printError("Error: No or invalid response received to RPC request. Method: " + methodName);
			result = BaseLib::Variable::createError(-1, "No response received.");
		}
		else result = response->response->arrayValue->at(1);

		{
			std::lock_guard<std::mutex> responseGuard(clientData->rpcResponsesMutex);
			clientData->rpcResponses.erase(packetId);
		}

		return result;
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return BaseLib::Variable::createError(-32500, "Unknown application error.");
}

void FlowsServer::sendResponse(PFlowsClientData& clientData, BaseLib::PVariable& scriptId, BaseLib::PVariable& packetId, BaseLib::PVariable& variable)
{
	try
	{
		BaseLib::PVariable array(new BaseLib::Variable(BaseLib::PArray(new BaseLib::Array{ scriptId, packetId, variable })));
		std::vector<char> data;
		_rpcEncoder->encodeResponse(array, data);
		send(clientData, data);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void FlowsServer::mainThread()
{
	try
	{
		int32_t result = 0;
		std::shared_ptr<BaseLib::FileDescriptor> clientFileDescriptor;
		while(!_stopServer)
		{
			if(!_serverFileDescriptor || _serverFileDescriptor->descriptor == -1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				getFileDescriptor();
				continue;
			}

			timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000;
			fd_set readFileDescriptor;
			int32_t maxfd = 0;
			FD_ZERO(&readFileDescriptor);
			{
				auto fileDescriptorGuard = GD::bl->fileDescriptorManager.getLock();
				fileDescriptorGuard.lock();
				maxfd = _serverFileDescriptor->descriptor;
				FD_SET(_serverFileDescriptor->descriptor, &readFileDescriptor);

				{
					std::lock_guard<std::mutex> stateGuard(_stateMutex);
					for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
					{
						if(i->second->closed) continue;
						if(i->second->fileDescriptor->descriptor == -1)
						{
							i->second->closed = true;
							continue;
						}
						FD_SET(i->second->fileDescriptor->descriptor, &readFileDescriptor);
						if(i->second->fileDescriptor->descriptor > maxfd) maxfd = i->second->fileDescriptor->descriptor;
					}
				}
			}

			result = select(maxfd + 1, &readFileDescriptor, NULL, NULL, &timeout);
			if(result == 0)
			{
				if(GD::bl->hf.getTime() - _lastGargabeCollection > 60000 || _clients.size() > GD::bl->settings.flowsServerMaxConnections() * 100 / 112) collectGarbage();
				continue;
			}
			else if(result == -1)
			{
				if(errno == EINTR) continue;
				_out.printError("Error: select returned -1: " + std::string(strerror(errno)));
				continue;
			}

			if (FD_ISSET(_serverFileDescriptor->descriptor, &readFileDescriptor) && !_shuttingDown)
			{
				sockaddr_un clientAddress;
				socklen_t addressSize = sizeof(addressSize);
				std::shared_ptr<BaseLib::FileDescriptor> clientFileDescriptor = GD::bl->fileDescriptorManager.add(accept(_serverFileDescriptor->descriptor, (struct sockaddr *) &clientAddress, &addressSize));
				if(!clientFileDescriptor || clientFileDescriptor->descriptor == -1) continue;
				_out.printInfo("Info: Connection accepted. Client number: " + std::to_string(clientFileDescriptor->id));

				if(_clients.size() > GD::bl->settings.flowsServerMaxConnections())
				{
					collectGarbage();
					if(_clients.size() > GD::bl->settings.flowsServerMaxConnections())
					{
						_out.printError("Error: There are too many clients connected to me. Closing connection. You can increase the number of allowed connections in main.conf.");
						GD::bl->fileDescriptorManager.close(clientFileDescriptor);
						continue;
					}
				}

				std::lock_guard<std::mutex> stateGuard(_stateMutex);
				if(_shuttingDown)
				{
					GD::bl->fileDescriptorManager.close(clientFileDescriptor);
					continue;
				}
				PFlowsClientData clientData = PFlowsClientData(new FlowsClientData(clientFileDescriptor));
				clientData->id = _currentClientId++;
				_clients[clientData->id] = clientData;
				continue;
			}

			PFlowsClientData clientData;
			{
				std::lock_guard<std::mutex> stateGuard(_stateMutex);
				for(std::map<int32_t, PFlowsClientData>::iterator i = _clients.begin(); i != _clients.end(); ++i)
				{
					if(i->second->fileDescriptor->descriptor == -1)
					{
						i->second->closed = true;
						continue;
					}
					if(FD_ISSET(i->second->fileDescriptor->descriptor, &readFileDescriptor))
					{
						clientData = i->second;
						break;
					}
				}
			}

			if(clientData) readClient(clientData);
		}
		GD::bl->fileDescriptorManager.close(_serverFileDescriptor);
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

PFlowsProcess FlowsServer::getFreeProcess(uint32_t maxThreadCount)
{
	try
	{
		if(GD::bl->settings.maxNodeThreadsPerProcess() != -1 && maxThreadCount > (unsigned)GD::bl->settings.maxNodeThreadsPerProcess())
		{
			GD::out.printError("Error: Could not get flow process, because maximum number of threads in flow is greater than the number of threads allowed per flow process.");
			return PFlowsProcess();
		}

		std::lock_guard<std::mutex> processGuard(_newProcessMutex);
		{
			std::lock_guard<std::mutex> processGuard(_processMutex);
			for(std::map<pid_t, PFlowsProcess>::iterator i = _processes.begin(); i != _processes.end(); ++i)
			{
				if(GD::bl->settings.maxNodeThreadsPerProcess() == -1 || i->second->nodeThreadCount() + maxThreadCount <= (unsigned)GD::bl->settings.maxNodeThreadsPerProcess())
				{
					return i->second;
				}
			}
		}
		_out.printInfo("Info: Spawning new flows process.");
		PFlowsProcess process(new FlowsProcess());
		std::vector<std::string> arguments{ "-c", GD::configPath, "-rl" };
		process->setPid(GD::bl->hf.system(GD::executablePath + "/" + GD::executableFile, arguments));
		if(process->getPid() != -1)
		{
			{
				std::lock_guard<std::mutex> processGuard(_processMutex);
				_processes[process->getPid()] = process;
			}

			std::mutex requestMutex;
			std::unique_lock<std::mutex> requestLock(requestMutex);
			process->requestConditionVariable.wait_for(requestLock, std::chrono::milliseconds(30000), [&]{ return (bool)(process->getClientData()); });

			if(!process->getClientData())
			{
				std::lock_guard<std::mutex> processGuard(_processMutex);
				_processes.erase(process->getPid());
				_out.printError("Error: Could not start new flows process.");
				return PFlowsProcess();
			}
			_out.printInfo("Info: Flows process successfully spawned. Process id is " + std::to_string(process->getPid()) + ". Client id is: " + std::to_string(process->getClientData()->id) + ".");
			if(_nodeEventsEnabled)
			{
				BaseLib::PArray parameters(new BaseLib::Array());
				BaseLib::PVariable response = sendRequest(process->getClientData(), "enableNodeEvents", parameters);
				if(response->errorStruct)
				{
					_out.printError("Error calling enableNodeEvents.");
				}
			}
			return process;
		}
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return PFlowsProcess();
}

void FlowsServer::readClient(PFlowsClientData& clientData)
{
	try
	{
		int32_t processedBytes = 0;
		int32_t bytesRead = 0;
		bytesRead = read(clientData->fileDescriptor->descriptor, &(clientData->buffer[0]), clientData->buffer.size());
		if(bytesRead <= 0) //read returns 0, when connection is disrupted.
		{
			_out.printInfo("Info: Connection to flows server's client number " + std::to_string(clientData->fileDescriptor->id) + " closed.");
			closeClientConnection(clientData);
			return;
		}

		if(bytesRead > (signed)clientData->buffer.size()) bytesRead = clientData->buffer.size();

		try
		{
			processedBytes = 0;
			while(processedBytes < bytesRead)
			{
				processedBytes += clientData->binaryRpc->process(&(clientData->buffer[processedBytes]), bytesRead - processedBytes);
				if(clientData->binaryRpc->isFinished())
				{
					std::shared_ptr<BaseLib::IQueueEntry> queueEntry(new QueueEntry(clientData, clientData->binaryRpc->getData(), clientData->binaryRpc->getType() == BaseLib::Rpc::BinaryRpc::Type::request));
					enqueue(0, queueEntry);
					clientData->binaryRpc->reset();
				}
			}
		}
		catch(BaseLib::Rpc::BinaryRpcException& ex)
		{
			_out.printError("Error processing packet: " + ex.what());
			clientData->binaryRpc->reset();
		}
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

bool FlowsServer::getFileDescriptor(bool deleteOldSocket)
{
	try
	{
		if(!BaseLib::Io::directoryExists(GD::bl->settings.socketPath().c_str()))
		{
			if(errno == ENOENT) _out.printCritical("Critical: Directory " + GD::bl->settings.socketPath() + " does not exist. Please create it before starting Homegear otherwise flows won't work.");
			else _out.printCritical("Critical: Error reading information of directory " + GD::bl->settings.socketPath() + ". The script engine won't work: " + strerror(errno));
			_stopServer = true;
			return false;
		}
		bool isDirectory = false;
		int32_t result = BaseLib::Io::isDirectory(GD::bl->settings.socketPath(), isDirectory);
		if(result != 0 || !isDirectory)
		{
			_out.printCritical("Critical: Directory " + GD::bl->settings.socketPath() + " does not exist. Please create it before starting Homegear otherwise flows won't work.");
			_stopServer = true;
			return false;
		}
		if(deleteOldSocket)
		{
			if(unlink(_socketPath.c_str()) == -1 && errno != ENOENT)
			{
				_out.printCritical("Critical: Couldn't delete existing socket: " + _socketPath + ". Please delete it manually. Flows won't work. Error: " + strerror(errno));
				return false;
			}
		}
		else if(BaseLib::Io::fileExists(_socketPath)) return false;

		_serverFileDescriptor = GD::bl->fileDescriptorManager.add(socket(AF_LOCAL, SOCK_STREAM | SOCK_NONBLOCK, 0));
		if(_serverFileDescriptor->descriptor == -1)
		{
			_out.printCritical("Critical: Couldn't create socket: " + _socketPath + ". Flows won't work. Error: " + strerror(errno));
			return false;
		}
		int32_t reuseAddress = 1;
		if(setsockopt(_serverFileDescriptor->descriptor, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseAddress, sizeof(int32_t)) == -1)
		{
			GD::bl->fileDescriptorManager.close(_serverFileDescriptor);
			_out.printCritical("Critical: Couldn't set socket options: " + _socketPath + ". Flows won't work correctly. Error: " + strerror(errno));
			return false;
		}
		sockaddr_un serverAddress;
		serverAddress.sun_family = AF_LOCAL;
		//104 is the size on BSD systems - slightly smaller than in Linux
		if(_socketPath.length() > 104)
		{
			//Check for buffer overflow
			_out.printCritical("Critical: Socket path is too long.");
			return false;
		}
		strncpy(serverAddress.sun_path, _socketPath.c_str(), 104);
		serverAddress.sun_path[103] = 0; //Just to make sure the string is null terminated.
		bool bound = (bind(_serverFileDescriptor->descriptor, (sockaddr*)&serverAddress, strlen(serverAddress.sun_path) + 1 + sizeof(serverAddress.sun_family)) != -1);
		if(_serverFileDescriptor->descriptor == -1 || !bound || listen(_serverFileDescriptor->descriptor, _backlog) == -1)
		{
			GD::bl->fileDescriptorManager.close(_serverFileDescriptor);
			_out.printCritical("Critical: Flows server could not start listening. Error: " + std::string(strerror(errno)));
		}
		if(chmod(_socketPath.c_str(), S_IRWXU | S_IRWXG) == -1)
		{
			_out.printError("Error: chmod failed on unix socket \"" + _socketPath + "\".");
		}
		return true;
    }
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    GD::bl->fileDescriptorManager.close(_serverFileDescriptor);
    return false;
}

void FlowsServer::startFlow(PFlowInfoServer& flowInfo, std::set<std::string>& nodes)
{
	try
	{
		if(_shuttingDown) return;
		PFlowsProcess process = getFreeProcess(flowInfo->maxThreadCount);
		if(!process)
		{
			_out.printError("Error: Could not get free process. Not executing flow.");
			flowInfo->exitCode = -1;
			return;
		}

		{
			std::lock_guard<std::mutex> processGuard(_currentFlowIdMutex);
			while(flowInfo->id == 0) flowInfo->id = _currentFlowId++;
		}

		_out.printInfo("Info: Starting flow with id " + std::to_string(flowInfo->id) + ".");
		process->registerFlow(flowInfo->id, flowInfo);

		PFlowsClientData clientData = process->getClientData();

		{
			std::lock_guard<std::mutex> nodeClientIdMapGuard(_nodeClientIdMapMutex);
			for(auto& node : nodes)
			{
				_nodeClientIdMap.emplace(node, clientData->id);
			}
		}

		BaseLib::PArray parameters(new BaseLib::Array{
				BaseLib::PVariable(new BaseLib::Variable(flowInfo->id)),
				flowInfo->flow
				});

		BaseLib::PVariable result = sendRequest(clientData, "startFlow", parameters);
		if(result->errorStruct)
		{
			_out.printError("Error: Could not execute flow: " + result->structValue->at("faultString")->stringValue);
			process->invokeFlowFinished(flowInfo->id, result->structValue->at("faultCode")->integerValue);
			process->unregisterFlow(flowInfo->id);

			{
				std::lock_guard<std::mutex> nodeClientIdMapGuard(_nodeClientIdMapMutex);
				for(auto& node : nodes)
				{
					_nodeClientIdMap.erase(node);
				}
			}

			return;
		}
		flowInfo->started = true;
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

// {{{ RPC methods
BaseLib::PVariable FlowsServer::registerFlowsClient(PFlowsClientData& clientData, BaseLib::PArray& parameters)
{
	try
	{
		pid_t pid = parameters->at(0)->integerValue;
		std::lock_guard<std::mutex> processGuard(_processMutex);
		std::map<pid_t, PFlowsProcess>::iterator processIterator = _processes.find(pid);
		if(processIterator == _processes.end())
		{
			_out.printError("Error: Cannot register client. No process with pid " + std::to_string(pid) + " found.");
			return BaseLib::Variable::createError(-1, "No matching process found.");
		}
		if(processIterator->second->getClientData())
		{
			_out.printError("Error: Cannot register client, because it already is registered.");
			return BaseLib::PVariable(new BaseLib::Variable());
		}
		clientData->pid = pid;
		processIterator->second->setClientData(clientData);
		processIterator->second->requestConditionVariable.notify_one();
		return BaseLib::PVariable(new BaseLib::Variable());
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return BaseLib::Variable::createError(-32500, "Unknown application error.");
}

BaseLib::PVariable FlowsServer::executePhpNode(PFlowsClientData& clientData, BaseLib::PArray& parameters)
{
	try
	{
		if(parameters->size() != 4) return BaseLib::Variable::createError(-1, "Method expects exactly three parameters.");

		std::string filename = parameters->at(1)->stringValue.substr(parameters->at(1)->stringValue.find_last_of('/') + 1);
		BaseLib::ScriptEngine::PScriptInfo scriptInfo(new BaseLib::ScriptEngine::ScriptInfo(BaseLib::ScriptEngine::ScriptInfo::ScriptType::node, parameters->at(0), parameters->at(1)->stringValue, filename, parameters->at(2)->integerValue, parameters->at(3)));
		GD::scriptEngineServer->executeScript(scriptInfo, false);
		return BaseLib::PVariable(new BaseLib::Variable());
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return BaseLib::Variable::createError(-32500, "Unknown application error.");
}

BaseLib::PVariable FlowsServer::nodeEvent(PFlowsClientData& clientData, BaseLib::PArray& parameters)
{
	try
	{
		if(parameters->size() != 3) return BaseLib::Variable::createError(-1, "Method expects exactly three parameters.");

		GD::rpcClient->broadcastNodeEvent(parameters->at(0)->stringValue, parameters->at(1)->stringValue, parameters->at(2));
		return BaseLib::PVariable(new BaseLib::Variable());
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return BaseLib::Variable::createError(-32500, "Unknown application error.");
}
// }}}

}