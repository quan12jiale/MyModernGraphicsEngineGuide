#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "QtConcurrent/qtconcurrentrun.h"

class MyRenderer : public IRenderer {
private:
public:
	MyRenderer()
		: IRenderer({QRhi::D3D11 }) {
	}
public:
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
	}
};

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiHelper::InitParams initParams;
	initParams.backend = QRhi::Implementation::D3D11;

	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}

