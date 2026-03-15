#include "pch.h"
#include "Components/TransformComponent.h"
#include <assert.h>

//QREntryPoint* entry_point;

class TestCase
{
public:
    TestCase(const std::string& test_name) : m_test_name(test_name)
    {
        std::cout << "Start Test Case: " << m_test_name << "\n";
    }

    void ExpectEqual(const Vector3& v1, const Vector3& v2)
    {
        constexpr float EPSILON = 0.001f;

        const bool x_is_equal = std::abs(v1.x - v2.x) < EPSILON;
        const bool y_is_equal = std::abs(v1.y - v2.y) < EPSILON;
        const bool z_is_equal = std::abs(v1.z - v2.z) < EPSILON;

        const bool is_equal = x_is_equal && y_is_equal && z_is_equal;
        if (!is_equal)
        {
            const std::string expected_text = "Expected: " + std::to_string(v1.x) + ", " + std::to_string(v1.y) + ", " + std::to_string(v1.z) + "\n";
            const std::string actual_text = "Actual: " + std::to_string(v2.x) + ", " + std::to_string(v2.y) + ", " + std::to_string(v2.z) + "\n";

            m_failed_expects.push_back(expected_text);
            m_failed_expects.push_back(actual_text);
        }
    }

    const std::vector<std::string>& GetFailedExpects() const { return m_failed_expects; }

private:
    std::string m_test_name;

    std::vector<std::string> m_failed_expects;
}; 

class TestBase {
public:
	TestBase() {}

    void SetTest(TestCase* test) { m_test = test; }

    template <typename T>
    void ExpectEqual(const T& v1, const T& v2) {
        m_test->ExpectEqual(v1, v2);
    }

private:
    TestCase* m_test = {};
};

template <typename TestClass>
class TestFixture {
public:
    TestFixture(const std::string& test_name) : m_test_name(test_name) {}

    template<typename... Input>
    TestFixture(const std::string& test_name, const Input&... inputs) : m_test_name(test_name) {
        std::cout << "STARTING TEST: " << m_test_name << "\n";
        (ExecuteTest(inputs), ...);

        int test_cases_failed = 0;
		for (const FailedExpects& failed_expects : m_failed_test_cases)
		{
			if (!failed_expects.empty())
			{
				test_cases_failed += 1;
			}
		}

        if (test_cases_failed > 0)
        {
            std::cout << "FAILED TEST: " << m_test_name << " WITH " << test_cases_failed << " TEST CASES FAILED\n";
        }
        else
        {
			std::cout << "PASSED TEST: " << m_test_name << "\n";
        }
    }

private:
    std::string m_test_name;
    TestClass m_test_to_run;

    using FailedExpects = std::vector<std::string>;
    std::vector<FailedExpects> m_failed_test_cases;

private:
    template <typename Input>
    void ExecuteTest(const Input& input) {
        TestCase test_case(input.description);

        m_test_to_run.SetTest(&test_case);

        m_test_to_run.Test(input);

        m_failed_test_cases.push_back(test_case.GetFailedExpects());

        const FailedExpects& failed_expects = m_failed_test_cases.back();

        if (!failed_expects.empty())
        {
            for (const std::string& failed_expect : failed_expects)
            {
                std::cout << failed_expect;
            }
            std::cout << "FAILED " << failed_expects.size() << " EXPECTS: " << m_test_name << "\n";
        }
    }
};

struct TestInput {
    std::string description;
    Vector3 input_rotation;
    Vector3 expected_rotation;
};

class TestRotation : public TestBase {
public:
    void Test(const TestInput& test_input)
    {
        TransformComponent transform{};

        transform.SetRotation(test_input.input_rotation);

        const Vector3 actual_rotation = TransformComponentInterface::GetDataFromWorldMatrix(transform).rotation;

        ExpectEqual(test_input.expected_rotation, actual_rotation);
    }
};

int main()
{
    constexpr auto Degrees60InRadians = DirectX::XM_PI / 3.0f;

    TestFixture<TestRotation> test_rotation("Rotation", 
        TestInput{.description = "Rotation x=0, y=0, z=0", .input_rotation = {0.0f, 0.0f, 0.0f}, .expected_rotation = {0.0f, 0.0f, 0.0f}},
        TestInput{.description = "Rotation x=PI, y=0, z=0", .input_rotation = {DirectX::XM_PI, 0.0f, 0.0f}, .expected_rotation = {0.0f, DirectX::XM_PI, DirectX::XM_PI}},
        TestInput{.description = "Rotation x=0, y=PI, z=0", .input_rotation = {0.0f, DirectX::XM_PI, 0.0f}, .expected_rotation = {0.0f, DirectX::XM_PI, 0.0f}},
        TestInput{.description = "Rotation x=0, y=0, z=PI", .input_rotation = {0.0f, 0.0f, DirectX::XM_PI}, .expected_rotation = {0.0f, 0.0f, DirectX::XM_PI}},
        TestInput{.description = "Rotation x=0, y=PI, z=PI", .input_rotation = {0.0f, DirectX::XM_PI, DirectX::XM_PI}, .expected_rotation = {0.0f, DirectX::XM_PI, DirectX::XM_PI}},
        TestInput{.description = "Rotation x=PI, y=0, z=PI", .input_rotation = {DirectX::XM_PI, 0.0f, DirectX::XM_PI}, .expected_rotation = {0.0f, DirectX::XM_PI, 0.0f}},
        TestInput{.description = "Rotation x=PI, y=PI, z=0", .input_rotation = {DirectX::XM_PI, DirectX::XM_PI, 0.0f}, .expected_rotation = {0.0f, 0.0f, DirectX::XM_PI}},
        TestInput{.description = "Rotation x=PI, y=PI, z=PI", .input_rotation = {DirectX::XM_PI, DirectX::XM_PI, DirectX::XM_PI}, .expected_rotation = {0.0f, 0.0f, 0.0f}},

        TestInput{.description = "Rotation x=PI/2, y=0, z=0", .input_rotation = {DirectX::XM_PI/2.0f, 0.0f, 0.0f}, .expected_rotation = {DirectX::XM_PI/2.0f, DirectX::XM_PI, DirectX::XM_PI}},
        TestInput{.description = "Rotation x=0, y=PI/2, z=0", .input_rotation = {0.0f, DirectX::XM_PI/2.0f, 0.0f}, .expected_rotation = {0.0f, DirectX::XM_PI/2.0f, 0.0f}},
        TestInput{.description = "Rotation x=0, y=0, z=PI/2", .input_rotation = {0.0f, 0.0f, DirectX::XM_PI/2.0f}, .expected_rotation = {0.0f, 0.0f, DirectX::XM_PI/2.0f}},
        TestInput{.description = "Rotation x=0, y=PI/2, z=PI/2", .input_rotation = {0.0f, DirectX::XM_PI/2.0f, DirectX::XM_PI/2.0f}, .expected_rotation = {0.0f, DirectX::XM_PI/2.0f, DirectX::XM_PI/2.0f}},
        TestInput{.description = "Rotation x=PI/2, y=0, z=PI/2", .input_rotation = {DirectX::XM_PI/2.0f, 0.0f, DirectX::XM_PI/2.0f}, .expected_rotation = {DirectX::XM_PI/2.0f, DirectX::XM_PI, -DirectX::XM_PI/2.0f}},
        TestInput{.description = "Rotation x=PI/2, y=PI/2, z=0", .input_rotation = {DirectX::XM_PI/2.0f, DirectX::XM_PI/2.0f, 0.0f}, .expected_rotation = {DirectX::XM_PI/2.0f, -DirectX::XM_PI/2.0f, DirectX::XM_PI}},
        TestInput{.description = "Rotation x=PI/2, y=PI/2, z=PI/2", .input_rotation = {DirectX::XM_PI/2.0f, DirectX::XM_PI/2.0f, DirectX::XM_PI/2.0f}, .expected_rotation = {DirectX::XM_PI/2.0f, -1.107149f, -1.107149f}},

        TestInput{ .description = "Rotation x=PI/4, y=0, z=0", .input_rotation = {DirectX::XM_PI / 4.0f, 0.0f, 0.0f}, .expected_rotation = {DirectX::XM_PI / 4.0f, 0.0f, 0.0f} },
        TestInput{ .description = "Rotation x=0, y=PI/4, z=0", .input_rotation = {0.0f, DirectX::XM_PI / 4.0f, 0.0f}, .expected_rotation = {0.0f, DirectX::XM_PI / 4.0f, 0.0f} },
        TestInput{ .description = "Rotation x=0, y=0, z=PI/4", .input_rotation = {0.0f, 0.0f, DirectX::XM_PI / 4.0f}, .expected_rotation = {0.0f, 0.0f, DirectX::XM_PI / 4.0f} },
        TestInput{ .description = "Rotation x=0, y=PI/4, z=PI/4", .input_rotation = {0.0f, DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f}, .expected_rotation = {0.0f, DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f} },
        TestInput{ .description = "Rotation x=PI/4, y=0, z=PI/4", .input_rotation = {DirectX::XM_PI / 4.0f, 0.0f, DirectX::XM_PI / 4.0f}, .expected_rotation = {DirectX::XM_PI / 4.0f, 0.0f, DirectX::XM_PI / 4.0f} },
        TestInput{ .description = "Rotation x=PI/4, y=PI/4, z=0", .input_rotation = {DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f, 0.0f}, .expected_rotation = {DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f, 0.0f} },
        TestInput{ .description = "Rotation x=PI/4, y=PI/4, z=PI/4", .input_rotation = {DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f}, .expected_rotation = {DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f, DirectX::XM_PI / 4.0f} },

        TestInput{ .description = "Rotation x=PI/3, y=0, z=0", .input_rotation = {Degrees60InRadians, 0.0f, 0.0f}, .expected_rotation = {Degrees60InRadians, 0.0f, 0.0f} },
        TestInput{ .description = "Rotation x=0, y=PI/3, z=0", .input_rotation = {0.0f, Degrees60InRadians, 0.0f}, .expected_rotation = {0.0f, Degrees60InRadians, 0.0f} },
        TestInput{ .description = "Rotation x=0, y=0, z=PI/3", .input_rotation = {0.0f, 0.0f, Degrees60InRadians}, .expected_rotation = {0.0f, 0.0f, Degrees60InRadians} },
        TestInput{ .description = "Rotation x=0, y=PI/3, z=PI/3", .input_rotation = {0.0f, Degrees60InRadians, Degrees60InRadians}, .expected_rotation = {0.0f, Degrees60InRadians, Degrees60InRadians} },
        TestInput{ .description = "Rotation x=PI/3, y=0, z=PI/3", .input_rotation = {Degrees60InRadians, 0.0f, Degrees60InRadians}, .expected_rotation = {Degrees60InRadians, 0.0f, Degrees60InRadians} },
        TestInput{ .description = "Rotation x=PI/3, y=PI/3, z=0", .input_rotation = {Degrees60InRadians, Degrees60InRadians, 0.0f}, .expected_rotation = {Degrees60InRadians, Degrees60InRadians, 0.0f} },
        TestInput{ .description = "Rotation x=PI/3, y=PI/3, z=PI/3", .input_rotation = {Degrees60InRadians, Degrees60InRadians, Degrees60InRadians}, .expected_rotation = {Degrees60InRadians, Degrees60InRadians, Degrees60InRadians} }
    );

    return 0;
}