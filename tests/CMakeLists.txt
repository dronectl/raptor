enable_testing()

# integration tests
add_subdirectory(bme280)
add_subdirectory(pwm)

# unittests
add_subdirectory(unit)
