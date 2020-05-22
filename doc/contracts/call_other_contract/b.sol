pragma solidity >=0.4.0 <0.7.0;

import "./abstract_a.sol";

contract B {
    function doYourThing(address addressOfA, uint arg1, uint arg2) public returns (uint result) {
        AbstractA my_a = AbstractA(addressOfA);
        return my_a.sum(arg1, arg2);
    }
}