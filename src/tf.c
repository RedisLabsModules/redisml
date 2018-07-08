#include "tf.h"

double TFPredict(const double * * features, Tensor *lr) {

	TF_SessionOptions* options = TF_NewSessionOptions();
	TF_Buffer* buffer = TF_NewBuffer();
	TF_Graph* graph = TF_NewGraph();
	TF_Status* status = TF_NewStatus();
	const char *tags[] = {"one","two","three"};

	TF_Session* session = TF_LoadSessionFromSavedModel(options, NULL, "", tags, 1, graph, NULL, status);

	//TF_Graph* graph = TF_NewGraph();

	//node1 = tf.constant(3.0, tf.float32)
	//node2 = tf.constant(4.0) # also tf.float32 implicitly
	//print(node1, node2)

//    double p = 0;
//    for (int i = 0; i < lr->clen; i++) {
//        p += features[i] * lr->coefficients[i];
//    }
//    return p + lr->intercept;
	  printf("Hello from TensorFlow C library version %s\n", TF_Version());

	return 1.1;
}

// private
// val score : Vector = > Double = (features) = > {
//   val m = margin(features) 1.0 / (1.0 + math.exp(-m))
// }

//double LogRegPredict(double *features, LinReg *lr) {
//    double m = LinRegPredict(features, lr);
//    double p = 1.0 / (1.0 + exp(-m));
//    return p;
//}
