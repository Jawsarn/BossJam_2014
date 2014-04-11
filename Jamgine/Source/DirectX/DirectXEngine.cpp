// Official libs
#include <exception>
#include <algorithm>  

#include <DirectX/DirectXColors.h>

// Project files
#include <Jamgine/Include/DirectX/DirectXEngine.h>
#include <Jamgine/Include/DirectX/DirectXShared.h>

namespace Jamgine
{
	namespace JDirectX
	{		
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 origin;
			DirectX::XMFLOAT2 offset;
			DirectX::XMFLOAT2 texture_offset;
			float  rotation;
			unsigned int flip;

			Vertex()
			{
			
			}

			Vertex(SpriteData input)
			{
				position = DirectX::XMFLOAT3(input.position.x, input.position.y, input.depth);
				origin	 = DirectX::XMFLOAT2(input.origin.x, input.origin.y);
				offset	 = DirectX::XMFLOAT2(input.width, input.height); //HARD CODEDDDED : TODO
				texture_offset = DirectX::XMFLOAT2(input.textureOffset.x, input.textureOffset.y);
				rotation = input.rotation;
				flip = (unsigned int)input.spriteEffect;
			}
		};

		DirectXEngine::DirectXEngine()
			: m_device(nullptr), m_deviceContext(nullptr), m_texture2DManager(nullptr)
		{
			DirectX::XMStoreFloat4x4(&m_view, DirectX::XMMatrixIdentity());
		}

		DirectXEngine::~DirectXEngine()
		{

		}

		ErrorMessage DirectXEngine::Initialize(void* p_data)
		{
			ErrorMessage l_errorMessage = J_OK;
			Jamgine::Data_Send l_data;
			try
			{
				l_data = *(Jamgine::Data_Send*)p_data;
			}
			catch (std::exception e)
			{
				return J_FAIL;
			}
			
			// Run regular init
			l_errorMessage = Initialize(l_data);
			if(l_errorMessage != J_OK)
				return J_FAIL;

			return l_errorMessage;
		}

		ErrorMessage DirectXEngine::Initialize(Jamgine::Data_Send p_data)
		{
			ErrorMessage l_errorMessage = J_OK;

			m_hInstance		= p_data.hInstance;
			m_clientWidth	= p_data.clientWidth;
			m_clientHeight	= p_data.clientHeight;
			
			// Register window
			l_errorMessage  = RegisterWindow(p_data);
				if(l_errorMessage != J_OK)
				return J_FAIL; 

			// Init swapchain and device
			l_errorMessage = InitializeSwapChain();
			if(l_errorMessage != J_OK)
				return J_FAIL; 

			// Create texturemanger
			l_errorMessage = Texture2DManager::CreateTexture2DManager(&m_texture2DManager);
			m_texture2DManager->Initialize(m_device);
			if(l_errorMessage != J_OK)
				return J_FAIL; 

			CreateDepthBuffer();
			InitializeRenderTarget();
			LoadShaders();
			CreateBuffer();
			SetViewport();
			CreateRasterizers();
			SetBlendState();

			return l_errorMessage;
		}

		void DirectXEngine::SetViewport()
		{
			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)m_clientWidth;
			vp.Height = (FLOAT)m_clientHeight;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			m_deviceContext->RSSetViewports(1, &vp);
		}

		void DirectXEngine::SetBlendState()
		{			
			HRESULT hr = S_OK;
			D3D11_BLEND_DESC l_blendStateDesc;
			l_blendStateDesc.AlphaToCoverageEnable			= false;
			l_blendStateDesc.IndependentBlendEnable			= false;
			l_blendStateDesc.RenderTarget[0].BlendEnable	= true;
			l_blendStateDesc.RenderTarget[0].SrcBlend		= D3D11_BLEND_SRC_ALPHA;
			l_blendStateDesc.RenderTarget[0].DestBlend		= D3D11_BLEND_INV_SRC_ALPHA;
			l_blendStateDesc.RenderTarget[0].BlendOp		= D3D11_BLEND_OP_ADD;
			l_blendStateDesc.RenderTarget[0].SrcBlendAlpha	= D3D11_BLEND_ONE;
			l_blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			l_blendStateDesc.RenderTarget[0].BlendOpAlpha	= D3D11_BLEND_OP_ADD;
			l_blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			hr = m_device->CreateBlendState(&l_blendStateDesc, &m_blendState);

			float l_blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
			m_deviceContext->OMSetBlendState(m_blendState, l_blendFactor, 0xffffffff);
		}


		HRESULT DirectXEngine::CreateRasterizers()
		{
			HRESULT hr = S_OK;
			D3D11_RASTERIZER_DESC desc;

			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_NONE;
			desc.FrontCounterClockwise = false;
			desc.DepthBias = 0;
			desc.SlopeScaledDepthBias = 0.0f;
			desc.DepthBiasClamp = 0.0f;
			desc.DepthClipEnable = false;
			desc.ScissorEnable = false;
			desc.MultisampleEnable = false;
			desc.AntialiasedLineEnable = false;

			hr = m_device->CreateRasterizerState(&desc, &m_rasterizerState);
			if (FAILED(hr))
				return hr;

			m_deviceContext->RSSetState(m_rasterizerState);

			return hr;
		}

		void DirectXEngine::CreateBuffer()
		{
			HRESULT hr = S_OK;

			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.Usage	 = D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
			hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_perFrameBuffer);

			bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
			hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_perTextureBuffer);
			bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
			hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_perWindowChangeBuffer);


			bufferDesc.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
			bufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
			bufferDesc.ByteWidth				= sizeof(Vertex) * 300000; // LOL fix this maybe
			hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_vertexBuffer);
			
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

			m_deviceContext->GSSetConstantBuffers(0, 1, &m_perFrameBuffer);			// maybe fix array? yes
			m_deviceContext->GSSetConstantBuffers(1, 1, &m_perTextureBuffer);
			m_deviceContext->VSSetConstantBuffers(2, 1, &m_perWindowChangeBuffer);

			DirectX::XMFLOAT4 PerWindowChange = DirectX::XMFLOAT4(m_clientWidth, m_clientHeight, 0, 0);
			m_deviceContext->UpdateSubresource(m_perWindowChangeBuffer, 0, nullptr, &PerWindowChange, 0, 0); // UPDATE

		}

		void DirectXEngine::InitializeRenderTarget()
		{
			HRESULT hr;
			ID3D11Texture2D* l_backBuffer;
			hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&l_backBuffer);
			hr = m_device	->CreateRenderTargetView(l_backBuffer, nullptr, &m_backBuffer_RTV);
			hr = m_device	->CreateUnorderedAccessView(l_backBuffer, nullptr, &m_backBuffer_UAV);

			m_deviceContext->OMSetRenderTargets(1, &m_backBuffer_RTV, m_depthStencilView);

			l_backBuffer->Release();
		}

		void DirectXEngine::CreateDepthBuffer()
		{
			HRESULT hr = S_OK;

			// Create depth stencil texture
			D3D11_TEXTURE2D_DESC descDepth;
			ZeroMemory(&descDepth, sizeof(descDepth));
			descDepth.Width				= m_clientWidth;
			descDepth.Height			= m_clientHeight;
			descDepth.MipLevels			= 1;
			descDepth.ArraySize			= 1;
			descDepth.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
			descDepth.SampleDesc.Count	= 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.Usage				= D3D11_USAGE_DEFAULT;
			descDepth.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags	= 0;
			descDepth.MiscFlags			= 0;

			hr = m_device->CreateTexture2D(&descDepth, nullptr, &m_depthStencil);

			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			ZeroMemory(&descDSV, sizeof(descDSV));
			descDSV.Format				= descDepth.Format;
			descDSV.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice	= 0;
			hr = m_device->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);

			//create states
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			// Depth test parameters
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		//	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

			// Stencil test parameters
			dsDesc.StencilEnable = true;	// YES?1
			dsDesc.StencilReadMask = 0xFF;
			dsDesc.StencilWriteMask = 0xFF;
			// Stencil operations if pixel is front-facing
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Stencil operations if pixel is back-facing
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


			hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
		}

		void DirectXEngine::LoadShaders()
		{
			HRESULT hr;
			m_shaderLoader->CreateGeometryShader(L"GeometryShader.hlsl", "GS", "gs_5_0", m_device, &m_geometryShader);
			m_shaderLoader->CreatePixelShader(L"PixelShader.hlsl", "PS", "ps_5_0", m_device, &m_pixelShader);

			D3D11_INPUT_ELEMENT_DESC l_desc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{ "ORIGIN", 0, DXGI_FORMAT_R32G32_FLOAT, 0,			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{ "OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 0,			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{ "TEXTURE_OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{"ROTATION", 0, DXGI_FORMAT_R32_FLOAT, 0,			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"FLIP", 0, DXGI_FORMAT_R32_UINT,				0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};

			unsigned int l_numElements = ARRAYSIZE(l_desc);
			m_shaderLoader->CreateVertexShaderWithInputLayout(L"VertexShader.hlsl", "VS", "vs_5_0", m_device, &m_vertexShader, l_desc, l_numElements, &m_inputLayout);

			m_deviceContext->VSSetShader(m_vertexShader,	nullptr, 0);
			m_deviceContext->PSSetShader(m_pixelShader,		nullptr, 0);
			m_deviceContext->GSSetShader(m_geometryShader,	nullptr, 0);
			m_deviceContext->IASetInputLayout(m_inputLayout);	
			m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			
			D3D11_SAMPLER_DESC sampler_desc;
			ZeroMemory(&sampler_desc, sizeof(sampler_desc));
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

			hr = m_device->CreateSamplerState(&sampler_desc, &m_samplerState);
			m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
		}


		ErrorMessage DirectXEngine::LoadTexture(Texture2DInterface** p_texture2DInterface, char* p_filePath)
		{
			return m_texture2DManager->GetTexture(p_texture2DInterface, p_filePath);
		}
	
		void DirectXEngine::Render(Position p_position,
			Position p_origin,
			Position p_textureOffset,
			Texture2DInterface* p_texture,
			SpriteEffect p_spriteEffect,
			float p_width,
			float p_height,
			float p_depth,
			float p_rotation)
		{
			m_renderData.push_back(SpriteData(p_position, p_origin, p_textureOffset, (Texture2D*)p_texture, p_spriteEffect, p_width, p_height, p_depth, p_rotation));
		}

		void DirectXEngine::Render(
			Position p_position,
			Position p_origin,
			Position p_textureOffset,
			Texture2DInterface* p_texture,
			float p_width,
			float p_height,
			float p_depth,
			float p_rotation) 
		{
			m_renderData.push_back(SpriteData(p_position, p_origin, p_textureOffset, (Texture2D*)p_texture, p_width, p_height, p_depth, p_rotation));
		}

		void DirectXEngine::Render(Position p_position,
			Position p_textureOffset,
			Texture2DInterface* p_texture,
			float p_width,
			float p_height,
			float p_depth)
		{
			m_renderData.push_back(SpriteData(p_position, p_textureOffset, (Texture2D*)p_texture, p_width, p_height, p_depth));
		}
		void DirectXEngine::Render(Position p_position, Position p_textureOffset,
			Texture2DInterface* p_texture,
			SpriteEffect p_spriteEffect,
			float p_width,
			float p_height,
			float p_depth)
		{
			m_renderData.push_back(SpriteData(p_position, p_textureOffset, (Texture2D*)p_texture, p_spriteEffect, p_width, p_height, p_depth));
		}
		
		void DirectXEngine::PostRender()
		{
			int max = m_renderData.size() - 1;
			if(max < 0)
				return; // DO NOTHING

			SortSprites();
		

			Vertex* a = new Vertex[max + 1];

			for (int i = 0; i < max + 1; i++)
			{
				a[i] = Vertex(m_renderData[i]);
			}

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			m_deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, a, (max+1) * sizeof(Vertex));
			//*(Vertex*)mappedResource.pData = *a;
			m_deviceContext->Unmap(m_vertexBuffer, 0);
			
			
			m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
			m_deviceContext->ClearRenderTargetView(m_backBuffer_RTV, DirectX::Colors::Yellow);

			// Update per frame buffer
			m_deviceContext->UpdateSubresource(m_perFrameBuffer, 0, nullptr, &m_view, 0, 0); // Transposse?
			
			unsigned int l_currentIndex = 0;
			unsigned int l_amount = 1;
//			unsigned int max = m_renderData.size()-1;
			for (unsigned int i = 0; i < max; i++)
			{
				if (m_renderData[i].texture == m_renderData[i + 1].texture)
					l_amount++;
				else
				{
					ID3D11ShaderResourceView* a = m_renderData[i].texture->GetShaderResourceView();	// change name
					m_deviceContext->PSSetShaderResources(0, 1, &a);

					DirectX::XMFLOAT4 PerTexture = DirectX::XMFLOAT4(1.0f, 1.0f, 0, 0);
					m_deviceContext->UpdateSubresource(m_perTextureBuffer, 0, nullptr, &PerTexture, 0, 0); // UPDATE
	
					m_deviceContext->Draw(l_amount,l_currentIndex);
					l_currentIndex += l_amount;
					l_amount = 1;
				}	
			}
			ID3D11ShaderResourceView* b = m_renderData[l_currentIndex].texture->GetShaderResourceView();	// change name
			m_deviceContext->PSSetShaderResources(0, 1, &b);

			DirectX::XMFLOAT4 PerTexture = DirectX::XMFLOAT4(1.0f, 1.0f, 0, 0);
			m_deviceContext->UpdateSubresource(m_perTextureBuffer, 0, nullptr, &PerTexture, 0, 0); // UPDATE
			m_deviceContext->Draw(l_amount, l_currentIndex);			


			m_swapChain->Present(0, 0);
			m_renderData.clear();
		}
				
		bool SortAlgorithm(SpriteData p_a, SpriteData p_b)
		{
			return (p_a.texture < p_b.texture);
		}
		
		void DirectXEngine::SortSprites()
		{
			std::sort(m_renderData.begin(), m_renderData.end(), &SortAlgorithm);
		}

		ErrorMessage DirectXEngine::RegisterWindow(Jamgine::Data_Send p_data)
		{
			ErrorMessage l_errorMessage = J_OK;

			WNDCLASS l_wndClass;
			l_wndClass.style         = CS_HREDRAW | CS_VREDRAW;
			l_wndClass.lpfnWndProc   = p_data.messageProc; 
			l_wndClass.cbClsExtra    = 0;
			l_wndClass.cbWndExtra    = 0;
			l_wndClass.hInstance     = m_hInstance;
			l_wndClass.hIcon         = LoadIcon(0, IDI_APPLICATION);
			l_wndClass.hCursor       = LoadCursor(0, IDC_ARROW);
			l_wndClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
			l_wndClass.lpszMenuName  = 0;
			l_wndClass.lpszClassName = L"Testgame";
			
			if (!RegisterClass(&l_wndClass))
				return J_FAIL;
							
			// Create window
			RECT rc = { 0, 0, m_clientHeight, m_clientWidth};
			//AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

			m_handle = CreateWindow(
				L"TestGame",
				L"Welcome to this window",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				rc.right - rc.left,
				rc.bottom - rc.top,
				NULL,
				NULL, 
				p_data.hInstance, 
				NULL);

			if(!m_handle)
				return J_FAIL;

			ShowWindow( m_handle, SW_SHOW);

			return l_errorMessage;
		}		

		ErrorMessage DirectXEngine::InitializeSwapChain()
		{
			ErrorMessage l_errorMessage = J_OK;

			DXGI_SWAP_CHAIN_DESC l_swapChainDesc;
			l_swapChainDesc.BufferDesc.Width					= m_clientWidth;
			l_swapChainDesc.BufferDesc.Height					= m_clientHeight;
			l_swapChainDesc.BufferDesc.RefreshRate.Numerator	= 60;
			l_swapChainDesc.BufferDesc.RefreshRate.Denominator	= 1;
			l_swapChainDesc.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
			l_swapChainDesc.SampleDesc.Count					= 1;
			l_swapChainDesc.SampleDesc.Quality					= 0;
			l_swapChainDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
			l_swapChainDesc.BufferCount							= 1;
			l_swapChainDesc.OutputWindow						= m_handle;
			l_swapChainDesc.Windowed							= true;
			l_swapChainDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
			l_swapChainDesc.Flags								= 0;

			D3D_FEATURE_LEVEL featureLevelsToTry[] = {
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0
			};

			UINT createDeviceFlags = 0;
			#ifdef _DEBUG
				createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			#endif
				D3D_FEATURE_LEVEL l_initiatedFeatureLevel;

			l_errorMessage = D3D11CreateDeviceAndSwapChain(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				createDeviceFlags,
				featureLevelsToTry,
				ARRAYSIZE(featureLevelsToTry),
				D3D11_SDK_VERSION,
				&l_swapChainDesc,
				&m_swapChain,
				&m_device,
				&l_initiatedFeatureLevel,
				&m_deviceContext);

			return l_errorMessage;
		}
	}
}