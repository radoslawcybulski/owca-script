#ifndef RC_OWCA_SCRIPT_AST_BASE_H
#define RC_OWCA_SCRIPT_AST_BASE_H

#include "stdafx.h"
#include "impl_base.h"
#include "ast_visitor.h"
#include "line.h"
#include "exec_buffer.h"

namespace OwcaScript {
	namespace Internal {
		class AstBase {
		public:
			const Line line;

			struct EmitInfo {
				class MaxCounter {
					unsigned int max = 0, current = 0;
				public:
					struct Popper {
						MaxCounter *self;
						Popper(MaxCounter &self) : self(&self) {}
						Popper(const Popper&) = delete;
						Popper(Popper &&o) : self(o.self) { o.self = nullptr; }
						Popper& operator=(const Popper&) = delete;
						Popper& operator=(Popper&& o) {
							if (this != &o) {
								if (self) {
									self->pop();
								}
								self = o.self;
								o.self = nullptr;
							}
							return *this;
						}
						~Popper() {
							if (self) self->pop();
						}
					};
					Popper push() {
						++current;
						if (current > max) {
							max = current;
						}
						return Popper(*this);
					}
					void pop(size_t s = 1) {
						assert(current >= s);
						current -= s;
					}
					bool empty() const { return current == 0; }
				};
				ExecuteBufferWriter code_writer;
				MaxCounter stack, states;
			};

			AstBase(Line line) : line(line) {}

			virtual ~AstBase() = default;

			virtual void emit(EmitInfo& ei) = 0;
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

			virtual void emit(EmitInfo& ei) = 0;
		};
	}
}

#endif
