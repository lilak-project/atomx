void check_event()
{
    auto run = new LKRun();
    run -> SetInputFile("test_output.root");
    run -> Init();
    run -> PrintEvent(0);
}
