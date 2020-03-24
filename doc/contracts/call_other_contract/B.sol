pragma solidity >=0.4.0 <0.7.0;

import "AbstractA.sol";

contract B {
    function doYourThing(address addressOfA, uint arg1, uint arg2) returns (uint) {
        AbstractA my_a = AbstractA(addressOfA);
        return my_a.f1(arg1, arg2);
    }
}