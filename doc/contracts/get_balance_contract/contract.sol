pragma solidity >=0.4.0 <0.7.0;

contract Balance {

    function getBalance() public view returns (uint256 balance){
        return msg.sender.balance;
    }

}
