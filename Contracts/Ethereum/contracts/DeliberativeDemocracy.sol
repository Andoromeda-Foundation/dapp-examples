pragma solidity ^0.4.24;

import "openzeppelin-solidity/contracts/ownership/Ownable.sol";
import "openzeppelin-solidity/contracts/lifecycle/Pausable.sol";

contract DeliberativeDemocracy is Ownable  {

    struct Proposal {
        address advocate; // 发起人
        uint256 act; 
        uint256 startTime;
        uint256 endTime;
    }

    struct Node {
        address member;
        uint256 tickets;
    }

    address public clientContractAddress;

    event Run(uint256 indexed _index, address indexed _member, uint256 indexed _tickets);
    event RunOut(uint256 indexed _index, address indexed _member, uint256 indexed _tickets);


    // 既然一token一票，某一个时刻只能投一个节点，那就直接用balances[]
    // mapping(address => uint256) public unused_balance; // 未使用的投票
    // mapping(address => uint256) public used_balance; // 使用的投票  
    mapping(address => bool) public isUsed; // 用户是否已经投票
    mapping(address => address) public candidate; // 我的候选人
    mapping(address => uint256) public ticket; // 得票数
    mapping(address => bool) public isMember; // 是否是议员
    mapping(address => uint256) public lastVoteTime; // 上次投票时间
    Node[21] public nodes; // 议员地址
    uint256 public minVotesIndex; // 当前节点最小票数对应的节点号
    uint256 public pledge;  // 一次提案需要质押的DGM数量


    // mapping(uint256 => address) public senate; // 议会 
    // mapping(uint256 => Proposal) public proposals; // 提案
    Proposal[] proposals;

    constructor(address client) public {
        clientContractAddress = client;
        pledge = 100 ether;        
    }

    function setClientAddress(address _client) public onlyOwner {
        clientContractAddress = _client;
    }

    function revote() public {
        if(isUsed[msg.sender]) {
            Client _client = Client(clientContractAddress);
            ticket[candidate[msg.sender]] -= _client.balanceOf(msg.sender);
            isUsed[msg.sender] = false;   
        }
    }

    function vote(address _candidate) public {
        revote();
        candidate[msg.sender] = _candidate;

        Client _client = Client(clientContractAddress);
        ticket[_candidate] += _client.balanceOf(msg.sender);

        isUsed[msg.sender] = true;
    }

    /*
    function challenge(address _voter) public {

    }

    function run() public {

    }

    function proposal() public {

    }

    function examine() public {

    }
    */

    // 参选议员 
    // 用户在参选议员前就可以接受投票
    // 如果参选没选上，他仍然可以继续参选
    function run() public {
        require(!isMember[msg.sender]);

        // 维护21个议员
        if (ticket[msg.sender] > nodes[minVotesIndex].tickets) {
            Node memory _node = Node(msg.sender, ticket[msg.sender]);
            nodes[minVotesIndex] = _node;

            uint256 _minTickets = nodes[0].tickets;
            uint256 _index = 0;
            for(uint256 i = 0; i < 21; i++) {
                if(nodes[i].tickets < _minTickets) {
                    _minTickets = nodes[i].tickets;
                    _index = i;
                }
            }
            
            minVotesIndex = _index;

            isMember[msg.sender] = true;
            emit Run(minVotesIndex, msg.sender, ticket[msg.sender]);
        }
    }

    // 退选议员
    // 退选议员，那么他所在那个位置的议员空掉，此时只有２０个议员(如果之前为２１)
    // 此时任何有票的人参选就可以选为议员。
    function runOut() public {
        require(isMember[msg.sender]);

        if (ticket[msg.sender] >= nodes[minVotesIndex].tickets) {
            for(uint256 i = 0; i < 21; i++) {
                if(nodes[i].member == msg.sender) {
                    nodes[i].member = 0x0;
                    nodes[i].tickets = 0;
                    minVotesIndex = i;

                    isMember[msg.sender] = false;
                    emit RunOut(i, msg.sender, ticket[msg.sender]);
                    break;
                }
            }
        }
    }

    function execute(string method, uint256 para) internal returns(bool){
        bytes4 methodId = bytes4(keccak256(abi.encodePacked(method)));
        return clientContractAddress.call(methodId, para);
        //if (!clientContractAddress.call(method, para)) {
        //    revert();
        //}
    }
    
    // 发起提案，

    function proposal(string method, uint256 para, uint256 dl) public {
        Client _client = Client(clientContractAddress);
        require(_client.balanceOf(msg.sender) - _client.lockTokens(msg.sender) >= _client.lockAmount_());

        _client.lockTokenAdd(msg.sender);

        

    }


    /*
    function withdraw() public {
       // Issuer issuer = Issuer(IssuerContractAddress);
       // issuer.transfer(msg.sender, unused_balance[msg.sender]);
    }

    function recharge(uint256 _value) public {
       // Issuer issuer = Issuer(IssuerContractAddress);
       // issuer.transferFrom(msg.sender, address(this), _value);
       // unused_balance[msg.sender] += _value;        
    }
    */
}

interface Client {
    function transferFrom(address _from, address _to, uint256 _value) external returns(bool);  
    function transfer(address _to, uint256 _value) external returns(bool);
    function balanceOf(address _owner) external view returns(uint256);
    function lockTokens(address _addr) external view returns(uint256);
    function lockAmount_() external view returns(uint256);
    function lockTokenAdd(address _addr) external;
    function lockTokenSub(address _addr) external;

    function setDividendFee(uint256 _dividendFee) external;
    function setCommunityFee(uint256 _communityFee) external;
    function setIncremental(uint256 _incremental) external;
    function setLockAmount(uint256 _amount) external;
    function lock(uint256 _lastTime) external;   
}
