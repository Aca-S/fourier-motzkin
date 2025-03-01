#include "theorem_prover.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    TheoremProver prover;
    std::cout << prover.is_theorem("!x.!y.!z.x < y & y < z => x < z") << std::endl;
    std::cout << prover.is_theorem("!x.!y.x < y => !z.x < z => z < y") << std::endl;
    std::cout << prover.is_theorem("?x.x>0 & x<0") << std::endl;
    std::cout << prover.is_theorem("!x.!y.!z.2*x < 3*y & 3*x < 2*y & 7*y < 5*z => 14*x < 10*z") << std::endl;
    std::cout << prover.is_theorem("!x.!y.x > 0 & y > 0 => x + y > 0") << std::endl;
    std::cout << prover.is_theorem("2*a + 3*b > c & a > b & c = 3*a & b < 0") << std::endl;
    std::cout << prover.is_theorem("!x.!y.!z.!u.(x < y & x + y = 2*z & y - x = u) => z + u > y") << std::endl;
    std::cout << prover.is_theorem("!x.!y.!z.!u.(x < 2*y & x + 2*y = z & y - z > u) => 5*y - 2*z > u") << std::endl;

    return 0;
}
