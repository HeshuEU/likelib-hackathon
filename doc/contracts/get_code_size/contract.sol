pragma solidity >=0.4.0 <0.8.5;

contract CodeItselfSize {
    function getCodeSize() public pure returns (uint size){
  	    assembly { size := codesize() }
    }
}

contract CodeAddressSize {
    function getCodeAddressSize(address addr) public view returns (uint size){
  	    assembly { size := extcodesize(addr) }
    }

}
