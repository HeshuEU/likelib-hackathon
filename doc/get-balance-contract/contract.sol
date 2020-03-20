pragma solidity >=0.4.0 <0.7.0;

contract Balance {
    address payable public stored_address;
    
    function setAddress() public{
        stored_address = msg.sender;
    }

    function getBalance() public view returns (uint256){
        return stored_address.balance;
    }
}
