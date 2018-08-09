pragma solidity ^0.4.24;


contract DeliberativeDemocracy {

    struct Proposal {
        address advocate; // 发起人
        uint256 act;
        uint256 startTime;
        uint256 endTime;
    }

    address public clientContractAddress;

    mapping(address => uint256) public unused_balance; // 未使用的投票
    mapping(address => uint256) public used_balance; // 使用的投票  
    mapping(address => address) public candidate; // 我的候选人
    mapping(address => uint256) public ticket; // 得票数
    mapping(address => uint256) public lastVoteTime; // 上次投票时间

    mapping(uint256 => address) public senate; // 议会 
    mapping(uint256 => Proposal) public proposals; // 提案

    constructor(address client) public {
        clientContractAddress = client;
    }

    function setClientAddress(address _client) public {
        if (clientContractAddress == 0x00) clientContractAddress = _client;
    }
 
    function revote() public {
        ticket[candidate[msg.sender]] -= used_balance[msg.sender];
        unused_balance[msg.sender] += used_balance[msg.sender];
    }

    function vote(address _candidate) public {
        revote();
        candidate[msg.sender] = _candidate;
        ticket[_candidate] += unused_balance[msg.sender];
        used_balance[msg.sender] = unused_balance[msg.sender];
        unused_balance[msg.sender] = 0;
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

    function execute(string method, uint256 para) public returns(bool){
        bytes4 methodId = bytes4(keccak256(method));
        return clientContractAddress.call(methodId, para);
        //if (!clientContractAddress.call(method, para)) {
        //    revert();
        //}
    }

    function withdraw() public {
       // Issuer issuer = Issuer(IssuerContractAddress);
       // issuer.transfer(msg.sender, unused_balance[msg.sender]);
    }

    function recharge(uint256 _value) public {
       // Issuer issuer = Issuer(IssuerContractAddress);
       // issuer.transferFrom(msg.sender, address(this), _value);
       // unused_balance[msg.sender] += _value;        
    }
}

interface Client {
    function transferFrom(address _from, address _to, uint256 _value) external;  
    function transfer(address _to, uint256 _value) external;
}

/**
 * @title SafeMath
 * @dev Math operations with safety checks that throw on error
 */
library SafeMath {

    /**
    * @dev Multiplies two numbers, throws on overflow.
    */
    function mul(uint256 a, uint256 b) internal pure returns (uint256) {
        if (a == 0) {
            return 0;
        }
        uint256 c = a * b;
        assert(c / a == b);
        return c;
    }

    /**
    * @dev Integer division of two numbers, truncating the quotient.
    */
    function div(uint256 a, uint256 b) internal pure returns (uint256) {
        // assert(b > 0); // Solidity automatically throws when dividing by 0
        uint256 c = a / b;
        // assert(a == b * c + a % b); // There is no case in which this doesn't hold
        return c;
    }

    /**
    * @dev Substracts two numbers, throws on overflow (i.e. if subtrahend is greater than minuend).
    */
    function sub(uint256 a, uint256 b) internal pure returns (uint256) {
        assert(b <= a);
        return a - b;
    }

    /**
    * @dev Adds two numbers, throws on overflow.
    */
    function add(uint256 a, uint256 b) internal pure returns (uint256) {
        uint256 c = a + b;
        assert(c >= a);
        return c;
    }
}