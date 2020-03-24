pragma solidity >=0.4.0 <0.7.0;

import "AbstractA.sol";

contract A is AbstractA {
    // implementation of f1
    function sum(uint arg1, uint arg2) returns(uint) {
        return arg1 + arg2;
    }
}
