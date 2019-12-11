// ----------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License
// ----------------------------------------------------------------------
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../../3rdParty/optional.h"
#include "../../Archive.h"
#include "../TfidfVectorizerFeaturizer.h"
#include "../Structs.h"
#include "../TestHelpers.h"
#include "../../Traits.h"

namespace NS = Microsoft::Featurizer;

using IndexMap = typename NS::Featurizers::TfidfVectorizerTransformer::IndexMap;

//todo: more tests will be added

TEST_CASE("string_standard_1") {
    using InputType       = std::string;
    using TransformedType = NS::Featurizers::TFIDFStruct;

    auto trainingBatches = 	NS::TestHelpers::make_vector<std::vector<InputType>>(
                                NS::TestHelpers::make_vector<InputType>("this is the first document"),
                                NS::TestHelpers::make_vector<InputType>("this document is the second document"),
                                NS::TestHelpers::make_vector<InputType>("and this is the third one"),
                                NS::TestHelpers::make_vector<InputType>("is this the first document")
                            );

    auto inferencingInput =  NS::TestHelpers::make_vector<InputType>("this is the first document");

    auto inferencingOutput = NS::TestHelpers::make_vector<TransformedType>( TransformedType(3,0.384085f),
                                                                            TransformedType(6,0.384085f),
                                                                            TransformedType(8,0.384085f),
                                                                            TransformedType(2,0.580286f),
                                                                            TransformedType(1,0.469791f)
                                                                          );

    // auto result = NS::TestHelpers::TransformerEstimatorTest(
    //         NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
    //         trainingBatches,
    //         inferencingInput
    //     );
    // for (auto i : result) {
    //     std::cout << i.dictionaryId << ":" << i.tfidf << std::endl;
    // }
    // std::cout << "--------------------------------" << std::endl;

    CHECK(
        NS::TestHelpers::TransformerEstimatorTest(
            NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
            trainingBatches,
            inferencingInput
        )== inferencingOutput
    );       
}

TEST_CASE("string_standard_2") {
    using InputType       = std::string;
    using TransformedType = NS::Featurizers::TFIDFStruct;

    auto trainingBatches = 	NS::TestHelpers::make_vector<std::vector<InputType>>(
                                NS::TestHelpers::make_vector<InputType>("this is the first document"),
                                NS::TestHelpers::make_vector<InputType>("this document is the second document"),
                                NS::TestHelpers::make_vector<InputType>("and this is the third one"),
                                NS::TestHelpers::make_vector<InputType>("is this the first document")
                            );

    auto inferencingInput =  NS::TestHelpers::make_vector<InputType>("this document is the second document");

    auto inferencingOutput = NS::TestHelpers::make_vector<TransformedType>( TransformedType(3,0.281089f),
                                                                            TransformedType(6,0.281089f),
                                                                            TransformedType(8,0.281089f),
                                                                            TransformedType(5,0.538648f),
                                                                            TransformedType(1,0.687624f)
                                                                          );

    // auto result = NS::TestHelpers::TransformerEstimatorTest(
    //         NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
    //         trainingBatches,
    //         inferencingInput
    //     );
    // for (auto i : result) {
    //     std::cout << i.dictionaryId << ":" << i.tfidf << std::endl;
    // }
    // std::cout << "--------------------------------" << std::endl;

    CHECK(
        NS::TestHelpers::TransformerEstimatorTest(
            NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
            trainingBatches,
            inferencingInput
        )== inferencingOutput
    );       
}

TEST_CASE("string_standard_3") {
    using InputType       = std::string;
    using TransformedType = NS::Featurizers::TFIDFStruct;

    auto trainingBatches = 	NS::TestHelpers::make_vector<std::vector<InputType>>(
                                NS::TestHelpers::make_vector<InputType>("this is the first document"),
                                NS::TestHelpers::make_vector<InputType>("this document is the second document"),
                                NS::TestHelpers::make_vector<InputType>("and this is the third one"),
                                NS::TestHelpers::make_vector<InputType>("is this the first document")
                            );

    auto inferencingInput =  NS::TestHelpers::make_vector<InputType>("and this is the third one");

    auto inferencingOutput = NS::TestHelpers::make_vector<TransformedType>( TransformedType(3,0.267104f),
                                                                            TransformedType(0,0.511849f),
                                                                            TransformedType(4,0.511849f),
                                                                            TransformedType(6,0.267104f),
                                                                            TransformedType(8,0.267104f),
                                                                            TransformedType(7,0.511849f)
                                                                          );
    // auto result = NS::TestHelpers::TransformerEstimatorTest(
    //         NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
    //         trainingBatches,
    //         inferencingInput
    //     );
    // for (auto i : result) {
    //     std::cout << i.dictionaryId << ":" << i.tfidf << std::endl;
    // }
    // std::cout << "--------------------------------" << std::endl;

    CHECK(
        NS::TestHelpers::TransformerEstimatorTest(
            NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
            trainingBatches,
            inferencingInput
        )== inferencingOutput
    );       
}

TEST_CASE("string_standard_4") {
    using InputType       = std::string;
    using TransformedType = NS::Featurizers::TFIDFStruct;

    auto trainingBatches = 	NS::TestHelpers::make_vector<std::vector<InputType>>(
                                NS::TestHelpers::make_vector<InputType>("this is the first document"),
                                NS::TestHelpers::make_vector<InputType>("this document is the second document"),
                                NS::TestHelpers::make_vector<InputType>("and this is the third one"),
                                NS::TestHelpers::make_vector<InputType>("is this the first document")
                            );

    auto inferencingInput =  NS::TestHelpers::make_vector<InputType>("is this the first document");

    auto inferencingOutput = NS::TestHelpers::make_vector<TransformedType>( TransformedType(3,0.384085f),
                                                                            TransformedType(6,0.384085f),
                                                                            TransformedType(8,0.384085f),
                                                                            TransformedType(2,0.580286f),
                                                                            TransformedType(1,0.469791f)
                                                                          );
    // auto result = NS::TestHelpers::TransformerEstimatorTest(
    //         NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
    //         trainingBatches,
    //         inferencingInput
    //     );
    // for (auto i : result) {
    //     std::cout << i.dictionaryId << ":" << i.tfidf << std::endl;
    // }
    // std::cout << "--------------------------------" << std::endl;

    CHECK(
        NS::TestHelpers::TransformerEstimatorTest(
            NS::Featurizers::TfidfVectorizerEstimator<std::numeric_limits<size_t>::max()>(NS::CreateTestAnnotationMapsPtr(1), 0),
            trainingBatches,
            inferencingInput
        )== inferencingOutput
    );       
}



