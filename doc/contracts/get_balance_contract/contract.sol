pragma solidity >=0.4.0 <0.7.6;

contract Balance {

    function getBalance() public view returns (uint256 balance){
        return msg.sender.balance;
    }

}
