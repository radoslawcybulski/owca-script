#ifndef RC_OWCA_SCRIPT_AST_BASE_H
#define RC_OWCA_SCRIPT_AST_BASE_H

#include "stdafx.h" 
#include "ast_visitor.h"
#include "line.h"
#include "exec_buffer.h"
#include "identifier_index.h"

namespace OwcaScript {
	class OwcaValue;

	namespace Internal {
		class AstCompiler;

		class AstBase {
		public:
			const Line line;
			struct TempInfo;

			struct EmitInfo {
				struct BreakLoopPositions {
					std::vector<ExecuteBufferWriter::JumpPlaceholder> break_positions;
					std::vector<ExecuteBufferWriter::JumpPlaceholder> continue_positions;
					std::uint8_t depth;
				};
				ExecuteBufferWriter code_writer;
				std::vector<BreakLoopPositions> break_loops;
				AstCompiler &compiler;

				struct PerFunctionInfo {
					std::vector<std::uint32_t> temporaries;
					std::uint32_t max_temporaries = 0;
					std::uint32_t local_variables = 0;
					bool generator = false;
				};

				PerFunctionInfo per_function;
				TempInfo allocate_temporary();
				void write_move(Line line, IdentifierIndex dest, IdentifierIndex src);
			};
			struct TempInfo {
				EmitInfo *ei = nullptr;
				IdentifierIndex index;

				bool is_temporary() const { return ei != nullptr; }
				void release() {
					if (!ei) return;
					ei->per_function.temporaries.push_back(index.index());
					ei = nullptr;
				}
				explicit TempInfo(IdentifierIndex index) : index(index) {
					assert(index.kind() != IdentifierIndexKind::Local);
				}
				TempInfo(EmitInfo &ei, IdentifierIndex index) : ei(&ei), index(index) {
					if (index.kind() != IdentifierIndexKind::Local || index.index() < ei.per_function.local_variables) {
						this->ei = nullptr; 
					}
				}
				~TempInfo() {
					release();
				}
				TempInfo(const TempInfo&) = delete;
				TempInfo(TempInfo&& other) : ei(other.ei), index(other.index) {
					other.ei = nullptr;
				}
				TempInfo& operator=(const TempInfo&) = delete;
				TempInfo& operator=(TempInfo&& other) {
					if (this != &other) {
						release();
						ei = other.ei;
						index = other.index;
						other.ei = nullptr;
					}
					return *this;
				}
			};
			AstBase(Line line) : line(line) {}

			virtual ~AstBase() = default;

			virtual void visit(AstVisitor&) = 0;
			virtual void visit_children(AstVisitor&) = 0;
		};

		class AstStat : public AstBase {
		public:
			using AstBase::AstBase;

			virtual void emit(EmitInfo& ei) = 0;
		};

		class AstExpr : public AstBase {
		public:
			using AstBase::AstBase;

			virtual TempInfo emit(EmitInfo& ei, std::optional<TempInfo> target = std::nullopt) = 0;
		};
	}
}

#endif
