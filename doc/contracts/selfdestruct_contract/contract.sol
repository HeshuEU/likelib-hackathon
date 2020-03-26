pragma solidity >=0.4.0 <0.7.0;

contract PayMe {
    uint256 coins_store;
    address payable target;

    constructor(address payable addr) public payable{
        target = addr;
    }

    function payMe() public payable returns(uint256 current_balance){
        coins_store += msg.value;
        return coins_store;
    }

    function initDelete() public {
        selfdestruct(target);
    }

}
