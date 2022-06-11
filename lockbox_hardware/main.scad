FN = 32;

$fn=FN;
make();

module make() {
    box();
    lid();
    cam();
}

module box() //make me
{
    cube(5);
}

module lid() //make me
{
    cube(5);
}

module cam() //make me
{
    cube(5);
}
