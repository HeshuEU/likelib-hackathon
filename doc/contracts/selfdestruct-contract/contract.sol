pragma solidity >=0.4.0 <0.7.0;

contract Kamikadze {

    address payable initializer;

    constructor() public payable {
        initializer = msg.sender;
    }

    function initDelete() public {
        selfdestruct(initializer);
    }

}
